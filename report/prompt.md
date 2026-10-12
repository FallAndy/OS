# Lab 1 提示词汇总

本文件汇总 Lab 1 实验过程中使用并整理后的主要提示词。根据实验指导书中“好的提示词”要求，以下提示词尽量采用四段式结构：

- `[PROMPT]`：说明任务目标、操作要求和输出要求。
- `[RELY]`：提供完成任务所需的最小可信上下文。
- `[GUARANTEE]`：明确本次任务必须交付的内容。
- `[SPECIFICATION]`：用可验证的方式描述输出应满足的要求。

由于 Lab 1 不要求补全内核代码，提示词主要用于实验指导书阅读、源码理解、练习作答、调试分析、报告撰写和交付物整理。

---

## Prompt 1：阅读实验指导书并梳理 Lab 1 学习目标

```markdown
[PROMPT]
**任务**：阅读当前工作区中的实验指导书镜像，重点阅读 `lab0`、`lab0.5` 和 `lab1` 相关页面，梳理 Lab 1 开始前需要理解的背景、环境要求、AI 协作方法和实验目标。

**操作要求**：请直接读取工作区中的指导书文件，不要依赖远程网页。阅读时优先关注实验目的、实验内容、练习题、项目组成、启动流程、GDB 调试和实验报告要求。

**输出要求**：用简洁、适合后续实验开展的语言总结：`lab0`、`lab0.5`、`lab1` 分别讲了什么，Lab 1 需要完成哪些核心任务，以及后续完成实验的建议路径。

[RELY]
- 指导书镜像目录：`docs/course-guide`
- Lab 0 页面目录：`docs/course-guide/lab0`
- Lab 0.5 页面目录：`docs/course-guide/lab0.5`
- Lab 1 页面目录：`docs/course-guide/lab1`
- Lab 1 重点页面：
  - `lab1_1_goals.html`
  - `lab1_2_1_exercise.html`
  - `lab1_2_2_file.html`
  - `lab1_3_1_layout.html`
  - `lab1_3_2_linkerscript.html`
  - `lab1_3_3_sbi_io.html`
  - `lab1_3_4_makeit.html`
  - `lab1_4_gdb.html`
  - `lab1_5_requirement.html`

[GUARANTEE]
必须输出以下内容：
1. `lab0` 的主要内容概括。
2. `lab0.5` 的主要内容概括。
3. `lab1` 的实验目标、实验内容和练习要求。
4. 完成 Lab 1 的推荐步骤。

[SPECIFICATION]
## 阅读总结
**Pre-Condition**:
- 工作区中已经存在实验指导书镜像。
- 需要从本地文件中读取内容。

**Post-Condition**:
- 输出应能帮助实验者理解 Lab 1 的整体目标和任务边界。
- 输出应明确 Lab 1 的两个练习分别关注什么。

**Requirements**:
- 不编造指导书中不存在的任务。
- 对 Lab 1 的总结应突出“最小可执行内核”“启动流程”“链接脚本”“OpenSBI 输出”“GDB 调试”这些关键词。
```

---

## Prompt 2：导入 Lab 1 代码并确认项目结构

```markdown
[PROMPT]
**任务**：将主机目录 `D:\学习、材料、资料\操作系统\lab1` 中的 Lab 1 实验代码复制到当前工作区，并确认复制后的代码结构是否完整。

**操作要求**：请先检查源目录是否存在，再检查其中是否包含 `Makefile`、`kern`、`libs`、`tools` 等关键内容。确认目标工作区中没有同名目录冲突后，再复制到当前工作区。

**输出要求**：说明是否成功看到源代码、复制后的目标路径是什么，并列出 Lab 1 的关键文件和目录。

[RELY]
- 主机源目录：`D:\学习、材料、资料\操作系统\lab1`
- 当前工作区：`\\wsl$\Ubuntu-22.04\home\fall\oslab\OS`
- Lab 1 代码应包含：
  - `Makefile`
  - `kern/init/entry.S`
  - `kern/init/init.c`
  - `kern/driver/console.c`
  - `kern/libs/stdio.c`
  - `libs/sbi.c`
  - `libs/printfmt.c`
  - `tools/kernel.ld`
  - `tools/function.mk`

[GUARANTEE]
必须完成或确认：
1. 源目录可读。
2. Lab 1 代码复制到当前工作区。
3. 复制后的目录中包含核心源码文件。
4. 不修改实验代码内容。

[SPECIFICATION]
## 代码导入
**Pre-Condition**:
- 主机路径下已经下载好 Lab 1 代码。
- 当前工作区可写。

**Post-Condition**:
- 当前工作区中出现 `lab1` 目录。
- `lab1` 目录中包含 `Makefile`、`kern`、`libs`、`tools`。

**Requirements**:
- 如果目标目录已经存在，应先说明情况，不要静默覆盖未知内容。
- 复制后需要用文件列表验证结果。
```

---

## Prompt 3：解释 Lab 1 每份关键代码的作用

```markdown
[PROMPT]
**任务**：阅读 Lab 1 源码，用简单易懂的语言解释每份关键代码在最小可执行内核中的作用，并梳理从 QEMU/OpenSBI 到内核输出的完整执行链路。

**操作要求**：请直接读取当前工作区中的 Lab 1 源码，不要只根据文件名猜测。解释时按照启动顺序组织：链接脚本、汇编入口、C 初始化、格式化输出、控制台封装、SBI 调用、基础库和 Makefile。

**输出要求**：给出面向初学者的代码导读，说明每个关键文件做什么、处于启动链路的哪个位置、与其他模块如何交互。最后给出建议阅读顺序。

[RELY]
- `tools/kernel.ld`：指定 `BASE_ADDRESS = 0x80200000` 和入口 `kern_entry`。
- `kern/init/entry.S`：定义 `kern_entry`、`bootstack`、`bootstacktop`。
- `kern/init/init.c`：定义 `kern_init`，清空 `.bss`，调用 `cprintf`，最后死循环。
- `kern/libs/stdio.c`：实现 `cprintf`、`vcprintf`、`cputchar`、`cputs`。
- `kern/driver/console.c`：将 `cons_putc` 封装到 `sbi_console_putchar`。
- `libs/sbi.c`：通过内联汇编设置寄存器并执行 `ecall`。
- `libs/printfmt.c`：负责格式化字符串解析。
- `libs/string.c`：提供 `memset` 等基础函数。
- `kern/mm/mmu.h`：定义 `PGSIZE = 4096`、`PGSHIFT = 12`。
- `kern/mm/memlayout.h`：定义 `KSTACKSIZE`。

[GUARANTEE]
必须解释以下文件或模块：
1. `Makefile`
2. `tools/kernel.ld`
3. `kern/init/entry.S`
4. `kern/init/init.c`
5. `kern/libs/stdio.c`
6. `kern/driver/console.c`
7. `libs/sbi.c`
8. `libs/printfmt.c`
9. `libs/string.c`
10. `libs/defs.h`、`libs/riscv.h`、`kern/mm/mmu.h`、`kern/mm/memlayout.h`

[SPECIFICATION]
## 代码导读
**Pre-Condition**:
- Lab 1 代码已经存在于当前工作区。
- QEMU 已能成功运行最小内核。

**Post-Condition**:
- 输出应能解释最小内核从 `0x80200000` 到 `kern_init` 再到 `cprintf` 的执行过程。
- 输出应明确 `cprintf -> vprintfmt -> cons_putc -> sbi_console_putchar -> ecall` 的输出链路。

**Requirements**:
- 解释应使用通俗语言，但不能牺牲关键技术事实。
- 不要求逐行解释所有代码，应突出和 Lab 1 启动流程相关的核心逻辑。
```

---

## Prompt 4：完成练习 1 的标准回答

```markdown
[PROMPT]
**任务**：根据 Lab 1 源码和实验指导书要求，回答练习 1：阅读 `kern/init/entry.S`，说明 `la sp, bootstacktop` 完成了什么操作、目的是什么；说明 `tail kern_init` 完成了什么操作、目的是什么。

**操作要求**：回答必须结合当前代码中的真实符号和内核启动流程。不要只解释单条汇编指令的字面意思，还要说明它们为什么出现在内核入口处。

**输出要求**：给出可直接写入实验报告的标准回答，语言清晰，结构完整。

[RELY]
- `kern/init/entry.S` 中定义：
  ```asm
  .globl kern_entry
  kern_entry:
      la sp, bootstacktop
      tail kern_init

  .section .data
      .align PGSHIFT
      .global bootstack
  bootstack:
      .space KSTACKSIZE
      .global bootstacktop
  bootstacktop:
  ```
- `KSTACKSIZE = KSTACKPAGE * PGSIZE`
- `KSTACKPAGE = 2`
- `PGSIZE = 4096`
- `kern_init` 在 `kern/init/init.c` 中定义，负责清空 `.bss`、输出启动信息并进入死循环。
- RISC-V 中 `sp` 是栈指针寄存器，栈通常向低地址增长。

[GUARANTEE]
回答必须包含：
1. `la sp, bootstacktop` 的作用。
2. 为什么要在进入 C 函数前设置栈。
3. `bootstack` 和 `bootstacktop` 的含义。
4. `tail kern_init` 的作用。
5. 为什么这里使用尾调用跳转是合理的。
6. 对这两条指令在启动流程中作用的总结。

[SPECIFICATION]
## 练习 1 答案
**Pre-Condition**:
- OpenSBI 已将控制权交给内核入口 `kern_entry`。
- 内核尚未完成 C 语言运行环境准备。

**Post-Condition**:
- `sp` 应指向内核启动栈顶。
- 控制流应从汇编入口转入 C 语言入口 `kern_init`。

**Requirements**:
- 必须说明 C 函数调用依赖栈。
- 必须说明 `kern_entry` 不需要返回，因此 `tail kern_init` 可以直接移交控制权。
- 回答应适合放入实验报告。
```

---

## Prompt 5：完成练习 2 的 GDB 调试记录与分析

```markdown
[PROMPT]
**任务**：根据 Lab 1 的 QEMU/GDB 调试要求，整理练习 2 的实验过程和标准回答：使用 GDB 跟踪 RISC-V 虚拟机从加电后的第一条指令开始，直到控制流进入 `0x80200000` 并开始执行内核第一条指令。

**操作要求**：请结合当前 Lab 1 的 Makefile、符号表和反汇编结果进行分析。需要说明应如何启动 `make debug` 和 `make gdb`，如何设置断点，如何观察 `pc`、`sp` 和 `0x80200000` 附近的指令。

**输出要求**：给出可直接写入实验报告的调试过程、观察结果和问题回答。

[RELY]
- `Makefile` 中 QEMU 调试目标包含 `-s -S`。
- `Makefile` 中内核启动参数：
  ```bash
  -kernel $(UCOREIMG)
  ```
- 当前符号表关键结果：
  ```text
  0000000080200000 kern_entry
  000000008020000a kern_init
  0000000080201000 bootstack
  0000000080203000 bootstacktop
  ```
- 当前反汇编关键结果：
  ```asm
  80200000: auipc sp,0x3
  80200004: mv    sp,sp
  80200008: j     8020000a <kern_init>
  ```
- QEMU virt 平台复位入口位于 `0x1000` 附近。
- OpenSBI 负责完成固件初始化并将控制权移交给内核。

[GUARANTEE]
回答必须包含：
1. `make debug` 和 `make gdb` 的用途。
2. 查看复位地址附近指令的 GDB 命令。
3. 在 `0x80200000` 设置断点的 GDB 命令。
4. 对 `0x1000` 初始执行地址的解释。
5. 对 `0x80200000` 和 `kern_entry` 对应关系的解释。
6. 对进入 `kern_init` 后行为的说明。

[SPECIFICATION]
## 练习 2 答案
**Pre-Condition**:
- 内核已经能够通过 `make qemu` 成功运行。
- 编译产物中存在 `bin/kernel`、`bin/ucore.img`、`obj/kernel.asm`、`obj/kernel.sym`。

**Post-Condition**:
- 能说明 CPU 加电后先从 `0x1000` 附近执行固件代码。
- 能说明控制流最终进入 `0x80200000` 的 `kern_entry`。
- 能说明 `kern_entry` 首先设置栈，再跳转到 `kern_init`。

**Requirements**:
- 调试记录应包含具体 GDB 命令。
- 结论必须和当前符号表、反汇编结果一致。
- 不得把 `0x1000` 误认为内核入口。
```

---

## Prompt 6：按照实验报告要求撰写单人实验报告

```markdown
[PROMPT]
**任务**：根据实验指导书的 Lab 1 报告要求、当前源码分析、练习 1 和练习 2 的答案，撰写一份 Markdown 格式实验报告，并按照单人实验报告口径组织内容。

**操作要求**：请使用已有实验报告模板作为结构参考，但不要保留小组分工等不适用于单人实验的空模板内容。报告内容应以文本说明为主，覆盖指导书明确要求的所有部分。

**输出要求**：在 `lab1/实验报告.md` 或交付目录 `report/report.md` 中形成完整 Markdown 报告。

[RELY]
- 实验名称：Lab 1: 比麻雀更小的麻雀（最小可执行内核）
- 姓名/学号：王帅2413995
- 指导书报告要求：
  1. 基于 Markdown 格式完成实验报告。
  2. 说明本章节整体逻辑线。
  3. 说明每个功能的核心函数或功能模块理解。
  4. 列出本实验重要知识点及其与 OS 原理知识点的对应关系。
  5. 列出 OS 原理中重要但本实验没有对应上的知识点。
  6. 回答所有练习中提出的问题。
- 已完成练习：
  - 练习 1：理解 `entry.S` 中的 `la sp, bootstacktop` 和 `tail kern_init`。
  - 练习 2：使用 GDB 验证启动流程。

[GUARANTEE]
报告必须包含：
1. 实验基本信息。
2. 实验目的。
3. 实验环境。
4. 本章整体逻辑分析。
5. 核心模块和函数理解。
6. 练习 1 答案。
7. 练习 2 调试过程和答案。
8. 测试与验证。
9. 重要 OS 知识点对应关系。
10. OS 原理中重要但本实验未覆盖的知识点。
11. 实验总结。

[SPECIFICATION]
## 实验报告
**Pre-Condition**:
- Lab 1 代码已经运行成功。
- 练习 1 和练习 2 的分析结论已经整理完成。

**Post-Condition**:
- 生成的报告可作为实验交付物提交。
- 报告不再强调小组分工，使用单人完成口径。

**Requirements**:
- 报告中引用的地址、符号和命令必须与当前 Lab 1 工程一致。
- 报告不得留下模板占位符，如“待补充”“学号1”“模块名称”等。
- 报告应保留可验证依据，例如 `0x80200000`、`kern_entry`、`bootstacktop`、`make qemu`、`make debug`、`make gdb`。
```

---

## Prompt 7：整理 Lab 1 交付物并提交 Git 分支

```markdown
[PROMPT]
**任务**：按照实验提交规范整理 Lab 1 交付物，并在 Git 仓库中创建 `lab1` 分支完成本地提交。

**操作要求**：交付目录必须只包含规范要求的 `code` 和 `report` 两个文件夹。`code` 文件夹包含实验实现后的源码；`report` 文件夹包含 `report.md`、`prompt.md` 和 `images` 文件夹。不要把指导书镜像目录或临时编译产物作为本次交付内容提交。

**输出要求**：完成目录整理、分支创建、暂存和提交，并说明提交后的分支名、提交号和未提交内容。

[RELY]
- Git 仓库根目录：`/home/fall/oslab/OS`
- 目标分支名：`lab1`
- 交付结构：
  ```text
  code/
    Makefile
    kern/
    libs/
    tools/
  report/
    report.md
    prompt.md
    images/
  ```
- 原始源码目录：`lab1`
- 报告源文件：`lab1/实验报告.md`

[GUARANTEE]
必须完成：
1. 创建或切换到 `lab1` 分支。
2. 创建 `code` 和 `report` 目录。
3. 将源码复制到 `code`，不包含 `bin`、`obj` 等编译产物。
4. 将实验报告复制为 `report/report.md`。
5. 将提示词汇总保存为 `report/prompt.md`。
6. 创建 `report/images` 文件夹，并确保可被 Git 跟踪。
7. 只提交 `code` 和 `report`。

[SPECIFICATION]
## Git 交付
**Pre-Condition**:
- Lab 1 报告和源码已经准备完成。
- 当前仓库可以创建分支和本地提交。

**Post-Condition**:
- Git 当前分支为 `lab1`。
- 本地存在一次包含 `code` 和 `report` 的提交。
- `docs` 和原始 `lab1` 工作目录不进入本次提交。

**Requirements**:
- 提交前必须检查 `git status` 和 `git diff --cached --stat`。
- 如果推送远端卡在认证或网络层，应保留本地提交并说明需要用户在有认证的终端执行 `git push -u origin lab1`。
```

