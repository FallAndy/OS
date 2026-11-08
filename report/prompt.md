# Lab 3 提示词与实际执行记录

本文整理 Lab 3 实际使用且能够从会话或附件核实的四份主要提示词。Prompt 1、Prompt 2 和 Prompt 4 没有独立的逐字文本附件，正文是依据会话历史恢复的结构化整理版，不声称逐字等同于原始输入；Prompt 3 存在原始附件，其逐字文本收录在本文附录 A。每份提示词之后单独记录真实执行过程，执行记录不是原 Prompt 的一部分。

提示词按照 Lab 0.5 要求使用 `[PROMPT]`、`[RELY]`、`[GUARANTEE]`、`[SPECIFICATION]` 四段式结构，强调任务能力、真实工程上下文、交付接口和可验证行为，而不是只给函数名或固定算法步骤。

## Prompt 1：Lab 3 初始框架与 Lab 2 成果安全整合（整理版）

> 来源状态：依据可核实的会话历史整理，保留任务、接口、边界和验证要求；不是逐字原始 Prompt。

### [PROMPT]

你是一名熟悉 RISC-V、ucore 操作系统、Git 版本管理和 C/汇编混合工程的内核开发专家。

现在需要将老师提供的 Lab 3 初始框架安全整合到已经完成的 Lab 2 项目中，为后续实现中断与异常处理功能做好准备。本轮任务仅涉及项目整合、兼容性检查和必要的构建验证，暂时不实现 Lab 3 的具体练习函数。

项目环境：

- Git 仓库：`/home/fall/oslab/OS`
- 现有实验代码：`/home/fall/oslab/OS/code/`
- 老师提供的 Lab 3 初始框架：`/home/fall/oslab/labcode-source/lab3/lab3_new/`
- 实验指导书：`/home/fall/oslab/OS/docs/course-guide/lab3/`
- 当前准备 `lab3` 分支，Lab 2 已完成物理内存初始化、Best-Fit、Buddy System、SLUB、QEMU/GDB 调试和报告。

操作要求：

1. 检查当前 Git 分支、提交记录和工作区状态。
2. 确认 `lab3` 分支以已完成的 Lab 2 成果为基础。
3. 对比当前 `code/` 与老师的 `lab3_new/` 目录结构。
4. 识别 Lab 3 新增文件、修改文件和与 Lab 2 存在冲突的文件。
5. 将 Lab 3 新增的中断和异常处理框架整合到现有项目。
6. 保留 Lab 2 已完成的物理内存管理代码。
7. 必要时调整构建文件，使 Lab 3 新增模块参与编译。
8. 完成后执行静态检查和构建验证。
9. 不修改 Lab 3 需要学生实现的练习函数。
10. 不修改老师提供的官方测试及评分标准。

禁止直接使用覆盖式复制命令将整个 `lab3_new/` 覆盖到 `code/`。如果 Git 工作区存在尚未提交的修改，必须先识别并保护，不能使用 `git reset --hard` 或 `git clean -fd` 等破坏性操作。

### [RELY]

可以依赖：

1. 已完成的 Lab 2 代码，包括 `pmm.c`、`pmm.h`、Best-Fit、Buddy、SLUB 及相关头文件和构建配置；这些成果应优先保留。
2. 老师实际提供的 Lab 3 初始框架，包括实际存在的 `trapentry.S`、`trap.c`、`trap.h`、`clock.c/h`、`intr.c/h` 等文件，不凭空假设文件。
3. 指导书 `lab3_2_1_exercise.html`、`lab3_2_2_files.html`、`lab3_3_2_trapentry.html`、`lab3_3_3_trap.html`、`lab3_4_lab_requirement.html`。
4. Git、Make、RISC-V GCC 和 QEMU 等现有工具。

### [GUARANTEE]

整合后必须保证：

- `code/` 中存在 Lab 3 所需的 `kern/driver/clock.c/h`、`intr.c/h` 和 `kern/trap/trapentry.S`、`trap.c`、`trap.h`。
- 保留 Best-Fit、Buddy、SLUB、物理内存初始化、地址映射和已验证启动逻辑。
- 以下接口及依赖存在：`SAVE_ALL`、`RESTORE_ALL`、`__alltraps`、`__trapret`、`struct trapframe`、`trap()`、`interrupt_handler()`、`exception_handler()`。
- 本轮只导入原始框架，不完成学生练习位置。

### [SPECIFICATION]

#### Pre-Condition

- 现有项目包含已完成的 Lab 2 代码。
- 老师 Lab 3 初始框架存在。
- 当前位于或能安全创建 `lab3` 分支。
- 已有修改得到保护。

#### Post-Condition

- Lab 3 新增源码和头文件正确导入。
- Lab 2 内存管理实现完整保留。
- 构建配置包含 Lab 3 模块。
- 函数声明、头文件依赖和引用关系完整。
- 老师的 Lab 3 练习占位符保留。
- Git 能明确显示新增或修改文件，没有意外删除 Lab 2 代码。

#### Case 1：新增文件

只存在于 Lab 3 框架的文件添加到对应目录，保持老师结构和注释。

#### Case 2：两个版本都有的文件

先比较差异，保留 Lab 2 已实现代码，只合入 Lab 3 必需新增功能，重点检查 `init.c`、`pmm.c`、头文件、Makefile 和工具脚本，不无条件覆盖。

#### Case 3：编译依赖冲突

定位头文件、未定义符号、重复定义或构建规则问题；只修复整合问题，不用空函数或伪实现提前完成练习。

#### Requirements

- 不破坏 Lab 1、Lab 2 Git 历史。
- 不覆盖 Lab 2 实现，不修改官方测试绕过错误。
- 不提前实现模块一或模块二，不猜测未知学号。
- 不进行无关重构，不执行 `git push`。
- 验证 `git status`、文件齐全性、Lab 2 核心实现、Makefile、`make -j2` 和 `git diff --stat`。

### 实际执行记录

- `lab3` 分支以 Lab 2 完成提交 `02b2c3f` 为基线。
- 逐文件导入 Trap、时钟、中断、同步和调试框架，没有覆盖式复制整个目录。
- 保留 `page_init()`、Best-Fit、Buddy、SLUB 和原地址映射；页分配公开接口增加局部中断保护以适应 Lab 3。
- Makefile 加入 Lab 3 源目录和评分入口，同时保留 Lab 2 可选构建开关。
- 整合后构建失败点来自尚未完成的 Lab 3 练习，而不是用假实现掩盖。

## Prompt 2：模块一——Trap 上下文恢复与 Trap 入口（整理版）

> 来源状态：依据可核实的会话历史整理，保留四段式功能规格；不是逐字原始 Prompt。

### [PROMPT]

你是一名精通 RISC-V RV64、ucore 操作系统内核、汇编语言和中断异常处理机制的操作系统开发专家。

当前项目已经完成 Lab 3 初始框架与 Lab 2 代码的安全整合。本轮任务是完成 Lab 3 第一个功能模块：Trap 上下文恢复与 Trap 入口功能。

- 项目根目录：`/home/fall/oslab/OS`
- 代码目录：`/home/fall/oslab/OS/code`
- 当前分支：`lab3`
- 目标文件：`code/kern/trap/trapentry.S`
- 需要实现：`RESTORE_ALL`、`__alltraps`

修改前阅读 `trapentry.S`、`trap.h`、`trap.c`、`libs/riscv.h` 和 Makefile，并参考 `lab3_2_1_exercise.html` 与 `lab3_3_2_trapentry.html`。严格依据真实数据结构和接口实现，不直接套用其他版本 ucore。本轮只实现模块一，不实现中断和异常处理函数。

### [RELY]

- `struct trapframe` 和 `struct pushregs` 的实际定义决定寄存器和 CSR 布局。
- 老师提供的 `SAVE_ALL` 保存 Trap 上下文，不得修改；特别注意 `x2/sp` 的特殊保存。
- `trap(struct trapframe *tf)` 是 C 语言处理入口。
- `__trapret` 与 `sret` 完成返回。
- 可使用 RV64 通用寄存器、ABI、`sstatus`、`sepc` 以及现有宏和常量。

### [GUARANTEE]

必须实现：

```asm
RESTORE_ALL
__alltraps
```

`RESTORE_ALL` 根据栈上 TrapFrame 恢复 Trap 前状态；`__alltraps` 保存上下文并按 ABI 把 TrapFrame 地址作为 `a0` 传给 `trap()`，返回后进入 `__trapret`。

必须保持 `SAVE_ALL`、`struct pushregs`、`struct trapframe`、`trap()` 和 `__trapret` 的对外约定不变。

### [SPECIFICATION]

#### RESTORE_ALL

**Pre-Condition**

- CPU 已进入 Trap 流程。
- `SAVE_ALL` 已把上下文保存到有效内核栈。
- TrapFrame 布局与真实结构体一致。
- C 处理函数已完成，准备恢复执行。

**Post-Condition**

- 通用寄存器、原始栈指针、`sstatus` 和 `sepc` 正确恢复。
- TrapFrame 栈空间得到正确处理。
- 不破坏尚未恢复的数据，可由后续 `sret` 返回正确位置。

**Case：正常恢复**

按照 `SAVE_ALL` 的实际槽位恢复上下文，使程序继续执行。

**Case：栈指针恢复**

区分 TrapFrame 当前基址与槽位中保存的原始 `sp`，不能把所有寄存器完全同序恢复。

**Requirements**

- 与 `SAVE_ALL` 完全对应。
- 遵守真实 TrapFrame 布局。
- 不提前破坏仍需使用的寄存器或栈信息。
- CSR 和栈恢复顺序正确。

#### `__alltraps`

**Pre-Condition**

- CPU 已发生由 S 模式处理的 Trap。
- `stvec` 已设置，内核栈有效，`trap()` 存在。

**Post-Condition**

- 上下文被保存，TrapFrame 地址正确传给 `trap()`。
- C 函数返回后进入 `__trapret`。
- 上下文可恢复并由 `sret` 返回。

**Case：正常进入**

建立有效 TrapFrame，调用 C 函数时满足 RISC-V ABI 和栈对齐。

**Case：处理完成**

进入恢复路径，不跳过保存、处理或恢复。

**Requirements**

- 正确处理 `SAVE_ALL`、`trap()`、`RESTORE_ALL`、`__trapret` 的关系。
- 不永久破坏原始上下文，不修改函数签名。
- 原则上只修改 `trapentry.S`，保留 `lab3` 注释并填写学号 2413995。
- 单独编译汇编，再尝试完整构建；模块二导致的失败要明确区分。
- 不 commit、不 push。

### 实际执行记录

- `RESTORE_ALL` 先恢复 `sstatus` 和 `sepc`，再恢复 `x1`、`x3` 至 `x31`，最后恢复 `x2/sp`。
- `__alltraps` 执行 `SAVE_ALL`、`mv a0, sp`、`jal trap`，返回点自然落在 `__trapret`。
- 36 个 RV64 槽位共 288 字节，保持 16 字节 ABI 对齐。
- 汇编单独编译通过；模块二完成后完整构建通过。
- 官方上下文测试实际验证了断点返回和除保留寄存器外的通用寄存器恢复，兼容启动下得到 35/35。

## Prompt 3：模块二——中断与异常处理（结构化阅读版）

> 来源状态：原始附件仍可读取。为保留现有执行记录，本节继续作为结构化阅读版；附件逐字全文见附录 A。

### [PROMPT]

你是一名精通 RISC-V RV64、ucore 操作系统、中断与异常处理、C 语言以及 QEMU 调试的操作系统内核开发专家。

当前项目已经完成 Lab 3 初始框架整合，并实现 `RESTORE_ALL` 和 `__alltraps`。本轮完成功能模块二：

```c
void interrupt_handler(struct trapframe *tf);
void exception_handler(struct trapframe *tf);
```

目标文件为 `code/kern/trap/trap.c`。修改前阅读 `trap.c`、`trap.h`、`clock.c/h`、`intr.c`、`riscv.h`、`sbi.h`，参考 Lab 3 练习、Trap 和时钟指导书。根据真实代码实现，不套用其他版本 ucore。

### [RELY]

- `struct trapframe` 的 `tf->cause` 保存 Trap 原因，`tf->epc` 保存返回位置，具体字段以真实 `trap.h` 为准。
- `trap()` 与 `trap_dispatch()` 已负责统一入口和分发。
- 可使用 `clock_set_next_event()`、`ticks`、`TICK_NUM`；`SHUTDOWN_AFTER` 若框架实际存在则控制关机条件。
- 可使用实际定义的 `IRQ_S_TIMER`、`CAUSE_ILLEGAL_INSTRUCTION`、`CAUSE_BREAKPOINT` 及其他中断编号。
- 可使用 `print_trapframe(tf)`、`cprintf()` 和 `libs/sbi.h` 中真实存在的关机接口，不自定义 SBI 机制。

### [GUARANTEE]

必须实现：

```c
void interrupt_handler(struct trapframe *tf);
void exception_handler(struct trapframe *tf);
```

保持 `struct trapframe`、`trap()`、`trap_dispatch()`、`clock_set_next_event()` 和 `print_trapframe()` 的接口不变。

### [SPECIFICATION]

#### `interrupt_handler()`

**Pre-Condition**

- CPU 已发生中断，Trap 入口已保存上下文。
- `trap_dispatch()` 已把有效 TrapFrame 分发到本函数。
- `tf->cause` 包含中断原因。

**Post-Condition**

- 正确识别中断类型。
- S 模式时钟中断设置下一次事件，每次使 `ticks` 增加一次并按周期输出。
- 若框架定义规定的关机条件，则达到条件后调用实际 SBI 关机接口。
- 其他中断执行提示或 TrapFrame 调试输出，之后可返回统一恢复流程。

**Case 1：S 模式时钟中断**

调用 `clock_set_next_event()`，更新 `ticks`；当 `ticks` 为 `TICK_NUM` 整数倍时按框架格式输出。不能只计数而不设置下一事件。

**Case 2：其他已知中断**

按老师要求完成类型识别和提示，不扩展复杂驱动。

**Case 3：未知中断**

调用 `print_trapframe(tf)` 提供诊断。

**Requirements**

- 按真实 `tf->cause` 和宏判断。
- 保证时钟持续触发和计数准确。
- 不改官方输出格式，不硬编码结果，不跳过处理。

#### `exception_handler()`

**Pre-Condition**

- CPU 已发生同步异常，TrapFrame 有效。
- `tf->cause` 与 `tf->epc` 保存异常信息。

**Post-Condition**

- 正确处理非法指令和断点，输出符合要求。
- 对需要继续执行的场景正确更新 `epc`，不重复触发同一指令。
- 其他异常输出 TrapFrame。

**Case 1：非法指令**

识别 `CAUSE_ILLEGAL_INSTRUCTION`，输出地址，根据实际指令长度调整 `epc`。

**Case 2：断点**

识别 `CAUSE_BREAKPOINT`，兼容 16 位压缩和 32 位普通指令长度，调整 `epc`。

**Case 3：其他异常**

调用 `print_trapframe(tf)`，不实现缺页、调度等范围外功能。

**Requirements**

- 不未经检查假定所有指令都是 4 字节。
- 不修改 `trap_dispatch()`、其他无关函数、模块一、Lab 2、官方测试和评分脚本。
- 完整编译并用 QEMU 验证持续时钟、非法指令和断点。
- 只有真实运行后才能报告通过；不 commit、不 push。

### 实际执行记录

- 中断处理先清除 `scause` 最高位再匹配中断编号；时钟中断先重装事件、再递增 `ticks`，每 100 次打印。
- 老师原始框架没有定义 `SHUTDOWN_AFTER`，专项审查后在 Makefile 中增加了 `SHUTDOWN_AFTER=<count>` 可选编译入口；默认测试行为不变，也没有猜测老师规定的常量值。
- 异常处理读取 `epc` 处低 16 位，最低两位为 `11` 时跳过 4 字节，否则跳过 2 字节。
- 首次编辑在标记前意外留下字符 `+`，真实编译报错后删除并重新构建成功。
- 普通 QEMU 持续输出 `100 ticks`；兼容启动下官方异常和定时器测试分别得到 25/25、40/40。

## Prompt 4：QEMU/OpenSBI 评分兼容性诊断（整理版）

> 来源状态：依据可核实的会话历史整理，保留诊断目标、证据要求和禁止项；不是逐字原始 Prompt。

### [PROMPT]

当前 Lab 3 两个功能模块已完成，在使用 `-kernel bin/ucore.img` 启动的隔离测试环境中三项功能测试合计 100/100，但直接执行 `make grade` 仍得到 0/100。

本轮任务是诊断并尽可能修复官方评分脚本与当前 QEMU/OpenSBI 环境之间的启动兼容问题。

### [RELY]

可以依赖：

- 项目 `/home/fall/oslab/OS`、代码目录 `code/`、当前 `lab3` 分支。
- 已完成的 Lab 3 内核、Makefile、老师的 `tools/grade.sh` 与 `tools/inject.py`。
- 当前 QEMU、OpenSBI、RISC-V 工具链和此前真实测试。
- 已知普通 `-kernel bin/ucore.img` 能启动；原始评分使用 generic loader，失败时 `Domain0 Next Address = 0x0`。

### [GUARANTEE]

必须完成：

1. 查明 `make grade` 的实际 QEMU 命令。
2. 判断失败在 QEMU、OpenSBI 还是内核入口。
3. 分析 `-device loader` 与 `-kernel` 的差异。
4. 检查镜像格式、加载地址和入口。
5. 核实 QEMU/OpenSBI 版本。
6. 不改测试逻辑和评分标准，寻找兼容启动方式。
7. 重新验证真实结果。

### [SPECIFICATION]

#### Pre-Condition

- 两模块和完整工程已完成，`-kernel` 可启动。
- 原始 `make grade` 无法正确评分。

#### Post-Condition

- 明确原始命令失败原因和可行兼容方法。
- 分清原始 `make grade` 与替代方案结果。
- 若不能修复原命令，提供可复现的全部官方测试执行方式。

#### Case 1：QEMU 启动参数

检查 generic loader 是否加载镜像、是否设置有效执行入口，并解释 `-kernel` 可启动的原因。

#### Case 2：OpenSBI 兼容

检查固件版本、启动模式和下一阶段入口传递；验证兼容配置。

#### Case 3：评分脚本与环境不兼容

- 保留官方原始脚本。
- 不修改断言、计分和预期输出。
- 使用环境配置或隔离兼容方式验证。
- 如实记录修改前后结果，不能声称原始命令通过。

#### Requirements

- 不伪造日志或硬编码输出。
- 不修改已通过的 Trap 或 Lab 2 代码。
- 不 commit、不 push。
- 最终报告版本、参数、修改文件、原始结果、替代结果和提交风险。

### 实际执行记录

- 实际版本为 QEMU 7.0.0、OpenSBI 1.0、RISC-V GCC 15.1.0。
- `bin/kernel` 是入口 `0xffffffffc0200000` 的 ELF；`bin/ucore.img` 是需要物理加载到 `0x80200000` 的裸二进制。
- GDB 在暂停的 generic loader 启动中观察到 `pc=0x1000`，并确认 `0x80200000` 已含内核首条指令；说明加载成功但 OpenSBI 下一阶段入口未设置。
- 原始方式输出 `Domain0 Next Address = 0x0`，内核未执行，真实 `make grade` 为 0/100。
- `-kernel bin/ucore.img` 使 OpenSBI 得到 `0x80200000`。隔离副本中只替换该参数，官方三项断言得到 100/100。
- 本轮报告重新生成并持久化了两种评分方式的完整日志，路径见 `../code/verification/README.md`。

## Prompt 完整性说明

| Prompt | 当前保存状态 |
|---|---|
| Prompt 1：框架整合 | 会话历史整理版，非逐字原文 |
| Prompt 2：模块一 | 会话历史整理版，非逐字原文 |
| Prompt 3：模块二 | 本节为阅读版，附录 A 保存可核实的原始附件全文 |
| Prompt 4：评分兼容诊断 | 会话历史整理版，非逐字原文 |

所有整理版均保留了接口、边界、Pre/Post-Condition、Case、Requirements、禁止项和验证目标，没有使用事后虚构内容冒充历史原文。报告撰写任务本身也不冒充功能实现阶段的 Prompt。

## 附录 A：模块二原始 Prompt 全文

以下内容逐字恢复自原始附件，仅外层增加四反引号围栏，附件内部 Markdown 未改写。

````text
# [PROMPT]

你是一名精通 RISC-V RV64、ucore 操作系统、中断与异常处理、C 语言以及 QEMU 调试的操作系统内核开发专家。

当前项目已经完成 Lab 3 初始框架整合，并实现功能模块一：

- `RESTORE_ALL`
- `__alltraps`

模块一已经通过单独编译，但完整内核因模块二尚未实现而无法完成构建。

**本轮任务：完成 Lab 3 功能模块二——中断与异常处理。**

项目目录：

`/home/fall/oslab/OS`

代码目录：

`/home/fall/oslab/OS/code`

当前分支：

`lab3`

目标文件：

`code/kern/trap/trap.c`

需要实现：

1. `interrupt_handler(struct trapframe *tf)`
2. `exception_handler(struct trapframe *tf)`

修改前请阅读当前源码及相关头文件，重点检查：

- `kern/trap/trap.c`
- `kern/trap/trap.h`
- `kern/driver/clock.c`
- `kern/driver/clock.h`
- `kern/driver/intr.c`
- `libs/riscv.h`
- `libs/sbi.h`

同时参考实验指导书：

- `docs/course-guide/lab3/lab3_2_1_exercise.html`
- `docs/course-guide/lab3/lab3_3_3_trap.html`
- `docs/course-guide/lab3/lab3_3_4_clock.html`

请根据真实代码实现功能，不得直接套用其他版本 ucore 的实现。

# [RELY]

本模块允许使用以下已有接口和数据结构。

## 1. TrapFrame

```c
struct trapframe;
```

该结构保存 Trap 发生时的处理器状态。

其中：

- `tf->cause`：记录 Trap 原因。
- `tf->epc`：记录 Trap 返回所使用的执行位置。

具体字段以当前 `trap.h` 中的实际定义为准。

## 2. Trap 分发接口

```c
void trap(struct trapframe *tf);
```

以及现有：

```c
trap_dispatch(tf);
```

用于将 Trap 分发到中断或异常处理函数。

## 3. 时钟相关接口

允许使用框架中已有的：

- `clock_set_next_event()`
- `ticks`
- `TICK_NUM`
- `SHUTDOWN_AFTER`

其中：

- `clock_set_next_event()` 用于安排下一次时钟事件。
- `ticks` 用于记录累计时钟中断次数。
- `TICK_NUM` 控制统计信息的输出周期。
- `SHUTDOWN_AFTER` 控制实验规定的关机条件。

所有常量和变量均以当前框架的真实定义为准。

## 4. 中断与异常编号

允许使用框架中已有的：

- `IRQ_S_TIMER`
- `CAUSE_ILLEGAL_INSTRUCTION`
- `CAUSE_BREAKPOINT`

以及其他已经定义的中断和异常编号。

不得自行创造新的编号或修改已有宏定义。

## 5. 调试与关机接口

允许使用：

- `print_trapframe(tf)`
- `cprintf()`
- 当前框架中已有的时钟统计输出函数
- `libs/sbi.h` 中实际提供的系统关机接口

不得自行定义新的 SBI 关机机制。

# [GUARANTEE]

本模块必须实现以下两个接口。

## 1. interrupt_handler()

```c
void interrupt_handler(struct trapframe *tf);
```

功能：

根据 TrapFrame 中记录的中断原因，识别并处理不同类型的中断。

其中，S 模式时钟中断必须能够持续触发，完成计数、统计信息输出以及规定条件下的系统关机。

## 2. exception_handler()

```c
void exception_handler(struct trapframe *tf);
```

功能：

根据 TrapFrame 中记录的异常原因，识别并处理非法指令异常、断点异常以及其他异常。

对于实验要求处理的异常，必须正确调整返回执行位置，使后续 Trap 返回能够正常进行。

## 3. 接口约束

必须保持以下接口不变：

- `struct trapframe`
- `trap()`
- `trap_dispatch()`
- `clock_set_next_event()`
- `print_trapframe()`

不得修改原有函数签名和数据结构布局。

# [SPECIFICATION]

## 一、interrupt_handler()

### Pre-Condition

1. CPU 已发生中断。
2. Trap 入口已经正确保存上下文。
3. `trap_dispatch()` 已将当前中断分发至 `interrupt_handler()`。
4. 参数 `tf` 指向有效的 TrapFrame。
5. `tf->cause` 包含有效的中断原因信息。

### Post-Condition

1. 正确识别当前中断类型。
2. 对 S 模式时钟中断完成相应处理。
3. 每次时钟中断使 `ticks` 增加一次。
4. 正确设置下一次时钟事件。
5. 按规定周期输出统计信息。
6. 达到规定条件后调用框架提供的关机接口。
7. 对其他中断执行实验要求的处理或调试输出。
8. 正常处理完成后能够返回统一的 Trap 返回流程。

### Case 1：S 模式时钟中断

当中断原因对应 `IRQ_S_TIMER` 时：

1. 调用 `clock_set_next_event()` 安排下一次时钟事件。
2. 更新 `ticks`，每次中断恰好增加一次。
3. 当 `ticks` 为 `TICK_NUM` 的正整数倍时，使用框架已有方式输出统计信息。
4. 统计输出达到 `SHUTDOWN_AFTER` 规定的次数后，调用框架实际提供的 SBI 关机接口。
5. 输出格式必须符合当前实验框架及官方测试要求。

特别注意：不得只累计 `ticks` 而不设置下一次时钟事件，否则无法保证中断持续触发。

### Case 2：其他已知中断

对于框架中已定义的其他中断类型，按照老师的实验要求完成类型识别和相应提示输出。

不要求实现复杂的硬件设备驱动。

### Case 3：未知中断

对于无法识别的中断：

```c
print_trapframe(tf);
```

输出当前 TrapFrame，以便调试。

### Requirements

1. 必须按照实际 `tf->cause` 和框架宏定义判断中断类型。
2. 必须确保时钟中断能够连续触发。
3. `ticks` 的更新次数必须正确。
4. 不得重复安排或遗漏关键时钟事件。
5. 不得修改官方输出格式。
6. 不得通过硬编码计数或跳过中断处理来满足测试。

## 二、exception_handler()

### Pre-Condition

1. CPU 已经发生同步异常。
2. Trap 入口已经保存上下文。
3. `trap_dispatch()` 已将异常分发至 `exception_handler()`。
4. 参数 `tf` 指向有效的 TrapFrame。
5. `tf->cause` 和 `tf->epc` 保存了相应的异常信息。

### Post-Condition

1. 正确识别异常类型。
2. 非法指令异常得到规定处理。
3. 断点异常得到规定处理。
4. 输出信息符合实验要求。
5. 对需要继续执行的场景，`tf->epc` 被正确调整。
6. 不会因错误的返回位置而反复触发同一个异常。
7. 对其他未要求处理的异常输出 TrapFrame 信息。

### Case 1：非法指令异常

当异常原因对应：

```c
CAUSE_ILLEGAL_INSTRUCTION
```

要求：

1. 识别当前异常。
2. 输出实验规定的非法指令异常信息。
3. 根据实际触发异常的指令长度及实验测试要求调整 `tf->epc`。
4. 保证 Trap 返回后能够从正确位置继续执行。

### Case 2：断点异常

当异常原因对应：

```c
CAUSE_BREAKPOINT
```

要求：

1. 识别断点异常。
2. 输出实验规定的断点异常信息。
3. 根据触发异常的实际指令长度调整 `tf->epc`。
4. 正确处理可能存在的 16 位压缩指令和 32 位普通指令。
5. 防止重复触发同一个断点异常。

### Case 3：其他异常

对于未要求具体处理的异常：

```c
print_trapframe(tf);
```

本实验不要求额外实现缺页恢复、进程调度或复杂异常恢复机制。

### Requirements

1. 必须依据真实异常原因进行分类。
2. 必须严格遵循官方规定的输出格式。
3. 必须根据实际指令编码、实验框架和测试要求确定 `epc` 调整量。
4. 不得未经检查就假定所有指令长度均为 4 字节。
5. 不得修改 `trap_dispatch()` 的原有逻辑。
6. 不得修改其他无关函数。

## 三、代码修改约束

1. 本轮原则上只修改 `kern/trap/trap.c` 中模块二要求完成的位置。
2. 保留老师原有的 `lab3` 注释。
3. 不修改已经完成的模块一汇编代码。
4. 不修改 Lab 2 的内存管理实现。
5. 不修改官方测试及评分脚本。
6. 不进行无关重构。
7. 不得添加虚假的测试结果或绕过异常处理。
8. 不执行 `git commit` 或 `git push`。

## 四、编译与测试

完成代码后：

1. 检查两个函数是否已正确实现。
2. 使用当前 Makefile 完整编译。
3. 如果编译失败，分析错误并进行必要修复。
4. 使用 QEMU 验证时钟中断是否持续触发。
5. 检查 `ticks` 是否正确增加。
6. 检查统计输出是否符合预期。
7. 检查系统是否能够在规定条件下关机。
8. 验证非法指令与断点异常处理。
9. 如果框架提供官方自动评分，执行 `make grade`。
10. 测试时合理设置超时，避免异常循环导致终端长期阻塞。

如果测试失败，应定位实际问题并修复，不得修改官方测试来获得通过结果。

注意：仅当真实运行得到相应结果时，才能报告测试通过。

## 五、最终交付

完成后请简洁输出：

1. 实际修改的文件。
2. `interrupt_handler()` 的实现逻辑。
3. `exception_handler()` 的实现逻辑。
4. `tf->epc` 的调整依据。
5. 完整编译结果。
6. QEMU 运行结果。
7. 官方自动评分结果。
8. 尚未解决的问题。

本轮只完成模块二及必要的验证，不编写实验报告，不提交或推送代码。

请直接开始修改当前项目。
````
