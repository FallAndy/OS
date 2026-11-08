# Lab 3 提示词

## Prompt 1：Lab 3 初始框架与 Lab 2 成果安全整合

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

## Prompt 2：模块一——Trap 上下文恢复与 Trap 入口

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

## Prompt 3：模块二——中断与异常处理

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

## Prompt 4：QEMU/OpenSBI 评分兼容性诊断

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
