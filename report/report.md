# 操作系统实验报告

## 实验基本信息

| 项目 | 内容 |
|---|---|
| 实验名称 | Lab 3：中断与异常处理 |
| 姓名 / 学号 | 王帅 / 2413995 |
| 完成方式 | 个人独立完成 |
| 当前分支 | `lab3` |

## 1. 实验目的与环境

### 1.1 实验目的

本实验在 Lab 2 物理内存管理的基础上，为 RISC-V 64 位 uCore 建立完整的 Trap 处理闭环：处理器发生中断或异常后，汇编入口保存上下文，将 TrapFrame 传给 C 语言处理函数；C 语言根据 `scause` 区分中断与异常并完成处理；最后汇编返回路径恢复上下文，通过 `sret` 返回被打断的执行流。

本实验实际完成了两个功能模块：

1. `RESTORE_ALL` 与 `__alltraps`：建立 Trap 入口、C 函数调用和上下文恢复机制。
2. `interrupt_handler()` 与 `exception_handler()`：处理持续时钟中断、非法指令、断点以及兜底情况。

### 1.2 实验环境

| 项目 | 实际环境 |
|---|---|
| 主机环境 | WSL2 Ubuntu 22.04 |
| 目标架构 | RISC-V RV64 |
| 交叉编译器 | `riscv64-unknown-elf-gcc 15.1.0` |
| 虚拟机 | QEMU 7.0.0，路径 `/opt/qemu/bin/qemu-system-riscv64` |
| 固件 | OpenSBI 1.0，Runtime SBI 0.3 |
| 正常启动镜像 | `bin/ucore.img` |
| 正常启动参数 | `-kernel bin/ucore.img` |

构建和运行证据保存在 `../code/verification/`。`../code/verification/build.log` 记录完整编译，`../code/verification/qemu_boot.log` 记录普通 QEMU 启动。

## 2. 实验整体设计

本实验按照指导书给出的三条主线逐步完成：

1. **Trap 上下文保存与恢复**：`SAVE_ALL` 将通用寄存器和关键 CSR 保存为 `struct trapframe`，`RESTORE_ALL` 做严格对称的恢复。
2. **Trap 入口与返回**：`__alltraps` 把当前 TrapFrame 地址作为 `a0` 传给 `trap()`，处理完成后经 `__trapret`、`RESTORE_ALL` 和 `sret` 返回。
3. **中断与异常处理**：`trap_dispatch()` 根据 `scause` 最高位区分中断与异常，分别调用 `interrupt_handler()` 和 `exception_handler()`。

整体控制流为：

```text
硬件产生 Trap
    -> stvec 指向 __alltraps
    -> SAVE_ALL 建立 TrapFrame
    -> a0 = sp，调用 trap(tf)
    -> trap_dispatch(tf)
       -> interrupt_handler(tf)
       或 exception_handler(tf)
    -> __trapret
    -> RESTORE_ALL
    -> sret
    -> 返回原执行流或 tf->epc 指定的位置
```

系统初始化顺序与该控制流匹配：`kern_init()` 先通过 `idt_init()` 设置 `stvec`，随后保留 Lab 2 的 `pmm_init()`，再由 `clock_init()` 设置第一个时钟事件并打开 `sie.STIE`，最后 `intr_enable()` 打开 `sstatus.SIE`。这样在全局中断打开前，Trap 入口、物理内存管理和第一个计时事件均已准备好。

## 3. 功能模块一：Trap 入口与上下文恢复

### 3.1 TrapFrame 布局

`struct trapframe` 由 32 个通用寄存器槽位和 4 个 CSR 相关字段组成：

```c
struct trapframe {
    struct pushregs gpr;  // x0 ~ x31
    uintptr_t status;     // sstatus
    uintptr_t epc;        // sepc
    uintptr_t badvaddr;   // stval（源码兼容名 sbadaddr）
    uintptr_t cause;      // scause
};
```

RV64 中 `REGBYTES` 为 8，因此 TrapFrame 总大小为 `36 * 8 = 288` 字节。288 是 16 的整数倍；只要进入 Trap 前栈已对齐，`jal trap` 时仍满足 RISC-V ABI 的 16 字节栈对齐要求。

### 3.2 `SAVE_ALL`

`SAVE_ALL` 是老师框架已有代码，本实验未修改。它先将 Trap 前的 `sp` 写入 `sscratch`，在栈上分配 36 个寄存器宽度的空间，然后保存除 `x2` 外的通用寄存器。随后：

```asm
csrrw s0, sscratch, x0
```

把原 `sp` 取到 `s0`，同时把 `sscratch` 清零，再把该值存入 TrapFrame 的 `x2` 槽位。最后保存 `sstatus`、`sepc`、`stval` 和 `scause`。因此槽位 2 保存的是 Trap 前的栈指针，而不是当前 TrapFrame 基址。

### 3.3 `RESTORE_ALL`

实现位置：`kern/trap/trapentry.S`，保留标记 `#lab3 function1 2413995`。

恢复顺序分为三步：

1. 从槽位 32、33 读取 `sstatus` 和 `sepc`，先写回 CSR。
2. 恢复 `x1` 以及 `x3` 到 `x31`。
3. 最后从槽位 2 恢复 `x2/sp`。

必须最后恢复 `sp`，因为在此之前所有加载都以当前 `sp` 为 TrapFrame 基址。若过早覆盖 `sp`，后续寄存器会从错误地址读取。CSR 也在恢复保存它们的临时寄存器前写回，避免丢失返回状态。

### 3.4 `__alltraps`、`__trapret` 与 `sret`

`__alltraps` 的核心逻辑为：

```asm
SAVE_ALL
mv a0, sp
jal trap
```

按照 RISC-V ABI，`a0` 是第一个参数，因此 `trap()` 接收到当前 TrapFrame 指针。`jal trap` 返回地址就是紧随其后的 `__trapret`，所以 C 函数返回后自然进入恢复流程。`__trapret` 执行 `RESTORE_ALL`，最后由 `sret` 根据恢复后的 `sstatus` 和 `sepc` 返回。

### 3.5 模块一规格

#### Pre-Condition

- `stvec` 已指向 `__alltraps`。
- CPU 已把 Trap 返回所需状态写入 Supervisor CSR。
- 当前内核栈有效，`SAVE_ALL` 可建立完整 TrapFrame。
- `struct trapframe` 的字段顺序与汇编槽位一致。

#### Post-Condition

- Trap 发生时的通用寄存器和 CSR 已完整保存并传给 `trap()`。
- `trap()` 返回后，所有已保存通用寄存器、`sstatus`、`sepc` 和原 `sp` 被恢复。
- TrapFrame 栈空间不再占用当前执行栈。
- CPU 可通过 `sret` 返回正确执行位置。

#### Case

- **正常 Trap 进入**：保存 36 个槽位，向 C 函数传入 `sp`。
- **C 处理完成**：从 `__trapret` 恢复上下文并执行 `sret`。
- **栈指针恢复**：使用槽位 2 中由 `sscratch` 取得的原 `sp`，并最后恢复。

#### Requirements

- 不修改 `SAVE_ALL`、TrapFrame 布局和 `trap()` 签名。
- 保存与恢复槽位必须严格对称。
- 调用 C 函数时栈必须满足 ABI 对齐。
- 不得在读取完成前覆盖 TrapFrame 基址或保存 CSR 的临时寄存器。

## 4. 功能模块二：中断与异常处理

### 4.1 `trap_dispatch()`

RISC-V 的 `scause` 最高位表示当前 Trap 是否为中断。源码把 `tf->cause` 转换为有符号 `intptr_t`：值小于零时调用 `interrupt_handler()`，否则调用 `exception_handler()`。该逻辑保持了老师框架原样。

### 4.2 `interrupt_handler()`

函数先清除 `cause` 最高位，取得中断编号，再按照 `IRQ_*` 宏分类。对 S 模式时钟中断执行：

```c
clock_set_next_event();
ticks++;
if (ticks % TICK_NUM == 0) {
    print_ticks();
}
```

先设置下一次事件能够保证时钟中断持续发生；每次中断只增加一次 `ticks`；每 100 次输出一次 `100 ticks`。其他已知中断输出类型信息；未知中断输出编号并调用 `print_trapframe(tf)`。

指导书要求累计输出达到 `SHUTDOWN_AFTER` 后关机，但老师提供的源码、头文件和原始构建配置中没有定义该宏，也没有给出规定次数。实现使用 `#ifdef SHUTDOWN_AFTER` 保留统计次数和 `sbi_shutdown()` 路径；本次提交进一步在 Makefile 中增加可选的 `SHUTDOWN_AFTER=<count>` 编译入口，不设置时保持官方测试原行为，获得老师规定的正整数后可显式启用。报告不猜测次数，也不把默认构建描述为已经执行自动关机。

### 4.3 `exception_handler()`

模块处理两种实验要求的同步异常：

- 非法指令：输出 `Illegal instruction at ...`。
- 断点：输出 `Breakpoint at ...`。

二者都需要更新 `tf->epc`，否则 `sret` 会回到同一条异常指令并再次触发 Trap。RISC-V 压缩指令最低两位不等于 `11`，长度为 2 字节；普通 32 位指令最低两位为 `11`。实现读取 `epc` 指向的低 16 位：

```c
return ((instruction & 0x3) == 0x3) ? 4 : 2;
```

然后把对应长度加到 `tf->epc`。其他异常调用 `print_trapframe(tf)`，本实验不实现缺页恢复等复杂处理。

### 4.4 模块二规格

#### `interrupt_handler()`

**Pre-Condition**

- `tf` 指向由汇编入口建立的有效 TrapFrame。
- `trap_dispatch()` 已确认 `scause` 最高位表示中断。
- 时钟驱动和 SBI 计时接口可用。

**Post-Condition**

- 中断类型被正确识别。
- S 模式时钟中断重新安排下一次事件，`ticks` 恰好增加一次。
- 每逢 `TICK_NUM` 的整数倍输出统计信息。
- 已知非时钟中断输出类型，未知中断输出完整 TrapFrame。
- 返回后可继续统一 Trap 返回流程。

**Case**

- **IRQ_S_TIMER**：重装定时器、计数和周期输出。
- **其他已知中断**：输出对应提示，不实现本实验范围外的设备驱动。
- **未知中断**：打印编号与 TrapFrame。

**Requirements**

- 中断编号必须清除 `scause` 最高位后再匹配。
- 不能只计数而不重装定时器。
- 不硬编码测试结果，不改变官方输出检查。

#### `exception_handler()`

**Pre-Condition**

- `tf` 指向有效 TrapFrame，`cause` 和 `epc` 已保存。
- `trap_dispatch()` 已确认当前为同步异常。

**Post-Condition**

- 非法指令和断点被识别并输出规定信息。
- `epc` 按实际 16 位或 32 位指令长度前移。
- 返回后不重复执行已经触发异常的指令。
- 未专门实现的异常输出 TrapFrame。

**Case**

- **CAUSE_ILLEGAL_INSTRUCTION**：输出地址并跳过实际长度的非法指令。
- **CAUSE_BREAKPOINT**：输出地址并跳过实际长度的断点指令。
- **其他异常**：仅提供诊断输出，不伪造恢复逻辑。

**Requirements**

- 不假设所有指令都是 4 字节。
- 不改变 `trap_dispatch()`、TrapFrame 或公开函数签名。
- 不扩展到进程调度、缺页恢复等本实验未要求功能。

## 5. AI 提示词设计

Prompt 归档保存在 `prompt.md`，包括：

1. Lab 3 初始框架与 Lab 2 成果安全整合（会话历史整理版）。
2. 模块一：Trap 上下文恢复与统一入口（会话历史整理版）。
3. 模块二：中断与异常处理（附录保存原始附件全文）。
4. QEMU/OpenSBI 评分兼容性诊断（会话历史整理版）。

提示词遵循 Lab 0.5 的四段式结构：

- **[PROMPT]**：描述模块能力、真实目标文件、操作边界和验证要求。
- **[RELY]**：只列工程中实际存在的结构体、宏、函数和文件，要求先读源码再实现。
- **[GUARANTEE]**：列出必须交付且签名不可改变的接口。
- **[SPECIFICATION]**：使用 Pre-Condition、Post-Condition、Case 和 Requirements 描述可验证行为。

这种设计没有把固定指令序列当成答案，而是约束保存/恢复关系、ABI、CSR、时钟重装、EPC 更新和不可修改范围，使 AI 可以选择具体实现，同时必须满足当前框架真实语义。模块粒度也按照“可独立实现和验证的一项功能”划分，而不是一个函数一个 Prompt。

## 6. 实验调试与问题解决

### 6.1 Lab 3 框架与 Lab 2 成果整合

最初 `lab3` 分支仍以已完成 Lab 2 的提交 `02b2c3f` 为基线，当前 `code/` 没有老师 Lab 3 的 Trap、时钟和中断框架。整合时没有覆盖式复制整个 `lab3_new`，而是逐文件比较并导入新增模块；`pmm.c` 中的 `page_init()`、Best-Fit、Buddy、SLUB 和地址映射均被保留。

启用中断后，物理页分配接口还使用 `local_intr_save()` / `local_intr_restore()` 包围管理器操作，避免中断打断链表和计数更新。Makefile 在保留 Lab 2 的 `PMM=buddy` 与 `SLUB_CHALLENGE=1` 可选构建开关的同时加入 `kern/trap`、`kern/sync` 和 Lab 3 评分入口。

### 6.2 模块一编译

模块一先单独编译 `trapentry.S`，确认汇编槽位、CSR 名称和 ABI 调用均可被当前工具链接受。完整构建在模块二未实现时无法完成，这是未完成依赖，不是通过空函数绕过的问题。模块二完成后，整个工程链接成功。

### 6.3 模块二编译错误

首次补丁在 `lab3 function2` 标记前意外留下一个字符 `+`，编译器立即报告语法错误。检查实际文件后删除该字符，重新编译成功。该问题由真实编译发现并修复，没有修改测试来规避错误。

### 6.4 `SHUTDOWN_AFTER` 与真实框架差异

模块二提示词和指导书都提到 `SHUTDOWN_AFTER`，但对实际源码、头文件、Makefile 和配置的全面搜索确认老师框架没有定义该宏，也没有给出数值。若直接给出默认值会编造实验要求；若默认启用任意次数的关机，又可能让持续到 `ticks=200` 的官方测试提前退出。

因此采用最小兼容方案：保留 `trap.c` 已有条件代码，在 Makefile 中提供 `make SHUTDOWN_AFTER=<count> ...` 的可选编译入口。默认构建不定义宏，官方测试不受影响；当老师确认规定次数后，传入该正整数即可编译自动关机路径。例如 `SHUTDOWN_AFTER=1` 仅用于验证机制时，会在第一次输出 `100 ticks` 后调用 `sbi_shutdown()`，该值不代表老师规定值。

### 6.5 QEMU/OpenSBI 评分兼容问题

普通 `make qemu` 使用：

```bash
-kernel bin/ucore.img
```

可以正常启动。但官方 `tools/grade.sh` 使用：

```bash
-device loader,file=bin/ucore.img,addr=0x80200000
```

在 QEMU 7.0.0/OpenSBI 1.0 下，GDB 观察到 loader 已把内核首字节写到 `0x80200000`，CPU 仍从复位向量 `0x1000` 进入 OpenSBI；OpenSBI 显示：

```text
Domain0 Next Address : 0x0000000000000000
```

因此失败发生在 OpenSBI 交接内核之前，Trap 代码没有执行。使用 `-kernel bin/ucore.img` 后，OpenSBI 得到：

```text
Domain0 Next Address : 0x0000000080200000
```

并进入内核。`-device loader,...,cpu-num=0` 会绕过正常 OpenSBI 交接，串口无输出，因此不是正确替代方案。最终在隔离副本中保留全部官方测试、断言和计分，只把启动参数替换为 `-kernel`，三项测试得到 100/100。仓库中的官方脚本未为此修改。

## 7. 实验测试与结果

### 7.1 编译和普通启动

执行：

```bash
cd /home/fall/oslab/OS/code
make -j2
timeout 6s make qemu
```

真实结果包括：

```text
+ ld bin/kernel
riscv64-unknown-elf-objcopy bin/kernel --strip-all -O binary bin/ucore.img
Domain0 Next Address      : 0x0000000080200000
(THU.CST) os is loading ...
memory management: best_fit_pmm_manager
check_alloc_page() succeeded!
++ setup timer interrupts
100 ticks
100 ticks
```

这同时证明 Lab 2 的默认 Best-Fit 管理器仍能初始化和通过自检，Lab 3 时钟中断能够持续返回并再次触发。

为验证指导书要求的自动关机能力，另外执行：

```bash
make clean
make SHUTDOWN_AFTER=1 qemu
```

真实运行在第一次输出 `100 ticks` 后由 `sbi_shutdown()` 正常结束，记录见 `../code/verification/shutdown_after_1.log`。这里的 1 只是机制测试值，不是老师规定次数；默认构建仍不定义宏，不影响官方持续时钟测试。

### 7.2 官方功能测试覆盖

| 用例 | 实际检查 | 兼容启动结果 |
|---|---|---|
| Test 1 | `ebreak` 进入 Trap；恢复测试汇编设置的寄存器；正确更新断点 EPC | PASS，35/35 |
| Test 2 | 4 字节非法指令返回；S 模式软件中断；未知中断 15；未知异常 20 的 TrapFrame | PASS，25/25 |
| Test 3 | 全局中断打开后持续重装定时器，等待 `ticks >= 200` | PASS，40/40 |

关键输出为：

```text
Breakpoint at 0xffffffffc0201ad4
[Test 1] Trap entry and breakpoint return: PASS (35/35)

Illegal instruction at 0xffffffffc0201a86
Supervisor software interrupt
Unknown interrupt: cause 15
[Test 2] Illegal instruction return: PASS (25/25)

100 ticks
100 ticks
[Test 3] Sustained supervisor timer: PASS (40/40, ticks=200)
```

测试中的具体地址由当前链接结果产生，不作为代码硬编码条件。

### 7.3 原始评分与兼容评分必须区分

| 方式 | QEMU 启动参数 | 真实结果 |
|---|---|---|
| 原始 `make grade` | `-device loader,file=bin/ucore.img,addr=0x80200000` | `Total Score: 0/100` |
| 隔离兼容评分 | `-kernel bin/ucore.img` | `Total Score: 100/100` |

原始 0/100 的三份串口日志只有 OpenSBI 信息，`Domain0 Next Address` 为 0；它们不能反映 Trap 实现失败。兼容评分运行的是同一份注入测试、同一组标志字符串和同一计分规则，区别仅是 QEMU 如何把下一阶段入口交给 OpenSBI。

证据文件：

- `../code/verification/original_make_grade.log`
- `../code/verification/original/.qemu.GRADE_LAB3_*.out`
- `../code/verification/compatible_make_grade.log`
- `../code/verification/compatible/.qemu.GRADE_LAB3_*.out`
- `../code/verification/grade_qemu7_compat.sh`
- `../code/verification/shutdown_after_1.log`

## 8. 实验对应的操作系统知识

### 8.1 中断与异常

中断通常由外部或异步事件触发，例如时钟到期；异常与当前指令同步，例如非法指令和断点。RISC-V 把二者统一表示为 Trap，通过 `scause` 最高位区分中断和异常，再用低位编号表示具体原因。

### 8.2 Trap 上下文

Trap 会打断任意位置的执行流。为了透明返回，内核必须保存所有可能被处理代码破坏的通用寄存器以及 `sepc`、`sstatus`、`scause`、`stval`。TrapFrame 是汇编与 C 语言共同遵守的内存 ABI；任何字段偏移不一致都会造成恢复错误。

Trap 上下文保存与进程上下文切换都涉及保存和恢复处理器状态，二者存在基础设施上的联系：时钟 Trap 可以形成调度时机，TrapFrame 也可以为调度器提供被中断执行流的寄存器快照。但本实验的 Trap 返回始终恢复同一条被中断执行流，目标是让一次中断或异常对原程序尽量透明；完整进程上下文切换则要把当前进程状态长期保存到进程控制块，选择另一个进程，恢复另一套寄存器、内核栈乃至地址空间。前者是进入和退出内核的机制，后者是改变 CPU 当前运行主体的调度动作，本实验只实现了前者。

### 8.3 RISC-V 特权级与 CSR

本实验由 S 模式处理 Trap。`stvec` 保存统一入口，`sepc` 保存返回位置，`sstatus` 保存返回特权级及中断状态，`scause` 保存原因，`stval` 提供异常附加信息，`sie` 和 `sstatus.SIE` 分别控制具体中断源与 S 模式全局中断。

### 8.4 `sret`

`sret` 不是普通函数返回。它使用恢复后的 Supervisor CSR 恢复特权状态和 PC。因此 `RESTORE_ALL` 不仅要恢复通用寄存器，还必须在 `sret` 前写回正确的 `sstatus` 和 `sepc`。

### 8.5 时钟中断

OpenSBI 的 `sbi_set_timer()` 安排一次未来事件。该事件是一次性的，所以每次处理时都必须调用 `clock_set_next_event()`。持续重装时钟是形成周期性时钟中断的关键，也是未来调度器获得时间片的基础。

时钟中断与进程调度的联系在于：周期性时钟为内核提供稳定的抢占检查点，调度器可以在这些检查点更新运行时间、减少时间片并决定是否切换进程。但时钟中断本身不等于调度。本实验的处理函数只重新设置定时器、累加 `ticks`、输出统计信息，并最终恢复原 TrapFrame；它没有就绪队列、调度策略、时间片归属或进程切换。因此本实验完成的是调度所需的时钟驱动和 Trap 入口，而不是进程调度器。

### 8.6 中断使能与关闭

`clock_init()` 设置 `sie.STIE` 允许 S 模式时钟中断源，`intr_enable()` 设置 `sstatus.SIE` 打开全局中断。Lab 2 的页分配接口使用局部关中断保护共享管理状态，体现了单核内核中最基本的临界区保护方式。

关中断与同步机制都可以防止临界区被并发执行流破坏。在当前单核实验中，保存本地中断状态并暂时清除 `SIE`，能够阻止同一 HART 上的中断处理程序打断页分配链表更新，退出临界区后再恢复原状态。区别在于，关本地中断只约束当前 HART，不能阻止其他核同时访问共享数据，也不能替代需要等待、所有权和内存顺序语义的锁；长时间关中断还会增加中断延迟。完整操作系统通常用自旋锁、互斥量、信号量等同步原语表达跨执行流或跨核互斥，并只在必要的短临界区结合关中断。本实验的做法是单核、短临界区条件下的基础保护，不是通用并发同步方案。

## 9. 本实验未涉及的重要知识

本实验建立了 Trap 基础设施，但尚未覆盖完整操作系统的以下部分：

- **完整进程上下文切换**：当前保存的是一次 Trap 的 CPU 状态，没有进程控制块之间的调度切换。
- **进程调度**：虽然有周期时钟，但没有就绪队列、时间片分配和调度策略。
- **用户态到内核态的系统调用闭环**：尚未实现用户进程、参数校验、系统调用表和返回用户态的完整路径。
- **缺页异常恢复**：其他异常仅打印 TrapFrame，没有按虚拟内存语义分配页面或终止进程。
- **完整设备中断驱动**：已区分外部中断编号，但没有 PLIC 初始化、设备确认和驱动处理。
- **多核并发与锁**：本实验使用局部关中断保护单核临界区，没有实现 SMP 下的自旋锁和跨核中断同步。

## 10. 实验总结

本实验在保留 Lab 2 内存管理成果的基础上，完成了 RISC-V uCore 的 Trap 保存、C 语言分发、具体中断/异常处理和上下文恢复闭环。模块一的核心是 TrapFrame 布局、恢复顺序和 ABI 一致性；模块二的核心是持续重装时钟、正确识别 `scause`、按真实指令长度更新 `epc`。

真实测试表明三个官方功能用例在兼容启动方式下全部通过，合计 100/100；`SHUTDOWN_AFTER=1` 的机制测试也证明可选自动关机路径能够正常退出，但老师规定的正式次数仍未在框架或指导书中给出。当前还存在评分环境风险：原始 `make grade` 在 QEMU 7.0.0/OpenSBI 1.0 下因 generic loader 没有向 OpenSBI 提供下一阶段入口而得到 0/100。报告和证据目录均保留这两种结果，没有把兼容测试冒充为原始评分通过。提交前需要确认正式关机次数及老师评分环境，或获得允许使用 `-kernel bin/ucore.img` 的说明。
