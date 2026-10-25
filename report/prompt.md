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


---

# Lab 2 提示词与实现记录

## Prompt 1：物理内存初始化与 Best-Fit 分配器

### [PROMPT]

**任务：** 完成 RISC-V ucore Lab 2 中的物理内存初始化功能和 Best-Fit 连续物理页分配功能，使代码能够在现有交叉编译环境下成功编译，并通过实验自带测试及 `make grade` 自动测试。

**操作要求：**

1. 直接在当前 Git 仓库的 `lab2` 分支中修改真实源文件，源码位于 `code/`。
2. 先实现 `page_init()`，再实现 Best-Fit。每一阶段都进行编译验证。
3. 严格遵循现有函数签名、数据结构和接口约定，不修改实验框架提供的 `basic_check()`、`best_fit_check()`、`default_check()`。
4. 保留 `lab2` 注释，将已完成位置的 `YOUR CODE` 替换为学号 `2413995`。
5. 不修改 `lab1` 分支，不删除现有报告，不使用硬编码的固定空闲内存范围替代设备树探测。
6. 实现完成后运行测试。对发现的问题实际修改代码、重新测试，不得仅通过修改测试函数使其通过。

**输出要求：** 总结修改的文件、核心实现逻辑、实际测试结果、遇到的问题和解决方法。把本次实际使用的提示词及其实现过程记录到 `report/prompt.md`，把真实测试结果记录到 `report/report.md` 中，保留后续补充其他任务的空间。

### [RELY]

请先阅读当前工程中的以下真实文件，使用其中的定义，不要自行猜测不存在的接口：

- `code/kern/mm/pmm.c`
- `code/kern/mm/pmm.h`
- `code/kern/mm/memlayout.h`
- `code/kern/mm/mmu.h`
- `code/kern/mm/default_pmm.c`
- `code/kern/mm/default_pmm.h`
- `code/kern/mm/best_fit_pmm.c`
- `code/kern/mm/best_fit_pmm.h`
- `code/kern/driver/dtb.c`
- `code/kern/driver/dtb.h`
- `code/libs/defs.h`
- `code/libs/list.h`
- `code/tools/test_lab2.c`
- `code/tools/grade.sh`

特别注意以下真实接口和约定：

```c
struct Page *pages;
size_t npage;
uint64_t va_pa_offset;
const size_t nbase;

uint64_t get_memory_base(void);
uint64_t get_memory_size(void);

struct Page *alloc_pages(size_t n);
void free_pages(struct Page *base, size_t n);
size_t nr_free_pages(void);
```

`pmm.h` 定义了 `struct pmm_manager`，包含 `init`、`init_memmap`、`alloc_pages`、`free_pages`、`nr_free_pages` 和 `check` 函数指针。

`memlayout.h` 定义了 `struct Page`、页面状态宏、`PHYSICAL_MEMORY_OFFSET`、`PHYSICAL_MEMORY_END` 等。

`defs.h` 定义了 `ROUNDUP()` 和 `ROUNDDOWN()`。

`default_pmm.c` 是可参考的完整 First-Fit 实现。

`best_fit_pmm.c` 中已有两个不可修改的测试函数，其中直接使用 `free_list` 和 `nr_free`，实现时必须保证这些标识符可用且符合原有测试约定。

### [GUARANTEE]

#### 功能一：物理内存初始化

在 `code/kern/mm/pmm.c` 中实现：

```c
static void page_init(void);
```

该函数由现有的 `pmm_init()` 调用。

#### 功能二：Best-Fit 分配器

在 `code/kern/mm/best_fit_pmm.c` 中提供完整的：

```c
const struct pmm_manager best_fit_pmm_manager;
```

实现其所有必需操作，具体内部函数名称可以自行设计，但必须符合现有框架接口。

修改 `pmm.c` 的 `init_pmm_manager()`，切换为：

```c
pmm_manager = &best_fit_pmm_manager;
```

不得删除原有 First-Fit 实现。

### [SPECIFICATION]

#### 1. page_init()

**Pre-Condition：** `dtb_init()` 已执行，物理内存管理器已通过 `init_pmm_manager()` 初始化。

**Post-Condition：** 正确设置 `va_pa_offset`、`npage`、`pages`，完成物理页面状态初始化，并将合法空闲页面交给当前内存管理器。

**有效设备树信息：**

- 使用 `get_memory_base()` 和 `get_memory_size()` 获取物理内存范围。
- 检查范围有效性、整数溢出及可管理地址上界。
- 根据链接符号 `end` 确定描述符数组位置，将 `pages` 放置在内核结尾之后的页对齐地址。
- 为 `[nbase, npage)` 中的物理页面建立描述符，初始全部标记为保留。
- 确保内核、对齐空隙和描述符数组占用的页面不会进入空闲页面集合。
- 只对剩余完整空闲页面调用 `init_memmap()`，并输出识别到的物理内存范围。

**异常和边界：**

- 设备树信息无效时通过 `panic()` 终止初始化。
- 没有可用完整页面时不得以零页面数调用 `init_memmap()`。
- 保证 `page2pa()` 与 `pa2page()` 的映射关系正确。

#### 2. Best-Fit

- 初始化空闲链表和空闲页计数。
- 分配时遍历所有可用块，选择不小于请求且大小最接近请求的块，必要时拆分。
- 释放时按物理地址顺序插入，并合并前后相邻空闲块。
- 覆盖单页、多页、分配失败、拆分、合并和计数恢复等边界。
- 不修改原有 `basic_check()` 和 `best_fit_check()`。
- `pmm_manager->check()` 必须实际执行既有检查。

#### 3. 测试与验收

依次执行 `make`、`make grade` 和 `make qemu`。目标验收标志为：

```text
memory management: best_fit_pmm_manager
check_alloc_page() succeeded!
[Test 1] Page Init: PASS
[Test 2] Best-Fit: PASS
Total Score: 20/20
```

不得在未实际通过时声称通过；完成后展示 `git diff --stat`，未经确认不得推送远程仓库。

### 实际实现过程

1. 阅读物理页描述符、页号转换、设备树接口、First-Fit 参考实现和自动测试脚本。
2. 在 `pmm.c` 中实现 `page_init()`，先通过单独编译 `obj/kern/mm/pmm.o` 验证。
3. 在 `best_fit_pmm.c` 中实现空闲链表初始化、地址有序插入、最优块选择、拆分、释放合并和空闲页计数，并注册 `best_fit_pmm_manager`。
4. 将 `pmm_manager` 切换到 Best-Fit，完整执行 `make`。
5. 首次 `make grade` 为 `0/20`。QEMU 输出显示 OpenSBI 的 Next Address 为 `0x0`，因此将旧式 loader 参数改为 `-kernel`。
6. 第二次 `make grade` 仍为 `0/20`。此时 Next Address 已是 `0x80200000`，但内核无输出；定位为启动页表缺少低地址恒等映射，`satp` 生效后无法继续取指。
7. 在 `entry.S` 中同时建立低地址恒等映射和高地址内核映射后，`make grade` 得到 `20/20`，`make qemu` 也输出正确的内存信息和分配器检查结果。
8. 最终未修改 `basic_check()`、`best_fit_check()` 或 `default_check()`，也未向远程仓库推送。

## Lab 2 后续提示词预留

后续将在本节继续补充 First-Fit 分析、Buddy System、测试设计、GDB 页表调试以及实验报告整理所使用的提示词。


---

## Prompt 2：Buddy System 分配模块实现

### [PROMPT]

**任务：** 在已经完成 `page_init()` 和 Best-Fit 分配器的 RISC-V ucore Lab 2 工程基础上，实现符合伙伴系统原理的连续物理页面分配器，并将其接入现有物理内存管理框架。

**操作要求：**

1. 在当前 `lab2` 分支修改真实源代码。
2. 新建 `kern/mm/buddy_pmm.c` 和 `buddy_pmm.h`，不得使用修改 Best-Fit 代码冒充伙伴系统的方式完成。
3. 实现真正的 2 的整数次幂块管理、按需拆分和同阶伙伴合并。
4. 保留现有 `page_init()`、First-Fit、Best-Fit 及老师的原始测试函数。
5. 使用可选择的编译配置接入 Buddy System。正常构建仍默认使用 Best-Fit，只有明确选择 Buddy 模式时才使用 `buddy_pmm_manager`。
6. 本阶段先完成分配器及接入，不要求自行创建完整的独立测试套件。测试将在第二份 Prompt 中完成。
7. 保留 Lab 2 相关注释，新增实现标明 `lab2` 和学号 `2413995`。
8. 不修改 `tools/test_lab2.c`、`tools/grade.sh`，不推送远程。

**输出要求：** 说明文件修改、算法实现、构建方式、测试情况和实际问题，并将使用的 Prompt 及实现过程追加到 `report/prompt.md`。

### [RELY]

实现前读取并遵循以下真实接口：

- `code/kern/mm/pmm.c`
- `code/kern/mm/pmm.h`
- `code/kern/mm/memlayout.h`
- `code/kern/mm/default_pmm.c`
- `code/kern/mm/best_fit_pmm.c`
- `code/kern/mm/best_fit_pmm.h`
- `code/libs/list.h`
- `code/Makefile`

`struct pmm_manager` 规定以下操作：

```c
void (*init)(void);
void (*init_memmap)(struct Page *base, size_t n);
struct Page *(*alloc_pages)(size_t n);
void (*free_pages)(struct Page *base, size_t n);
size_t (*nr_free_pages)(void);
void (*check)(void);
```

`struct Page` 使用现有的 `ref`、`flags`、`property`、`page_link` 字段。不得假设存在动态内存库、Linux 内核 API 或其他未提供接口。

### [GUARANTEE]

必须新增并注册：

```c
extern const struct pmm_manager buddy_pmm_manager;
```

它必须实现 `init`、`init_memmap`、`alloc_pages`、`free_pages`、`nr_free_pages` 和 `check`。

修改 `pmm.c` 和必要构建配置，使 Buddy 模式可被显式选择，同时不得影响默认 Best-Fit 构建和评分。

### [SPECIFICATION]

#### 1. 初始化

初始化所有阶的空闲块管理结构，并将空闲页计数置零。

#### 2. 注册空闲物理页

将 `page_init()` 提供的可用区间划分为物理页号对齐、大小为 2 的整数次幂的块。区域起点未对齐或总页数不是 2 的整数次幂时仍须正确划分，不得越过传入边界或登记保留页面。

#### 3. alloc_pages(n)

- `n > 0`。
- 非 2 的整数次幂请求向上取整。
- 优先使用目标阶空闲块；只有更高阶块时逐级拆分。
- 无法满足时返回 `NULL`，且不修改管理状态。
- 记录实际分配块大小并避免整数溢出。

#### 4. free_pages(base, n)

- 参数必须对应之前成功分配且尚未释放的块。
- 使用伙伴规则识别同阶伙伴，不能合并任意相邻块。
- 同阶伙伴空闲时递归向更高阶合并。
- 不跨越页面描述符范围或保留区合并。
- 空闲页计数必须等于所有空闲块大小之和。

#### 5. 检查功能

`buddy_pmm_manager.check()` 必须执行实际结构与分配检查，不得仅打印成功。

#### 6. 验收

至少执行：

```bash
make clean
make PMM=buddy
make PMM=buddy qemu

make clean
make grade
```

Buddy 模式应成功启动并使用新管理器；恢复默认模式后 Best-Fit 自动评分仍应为 `20/20`。切换模式前必须清理旧目标文件，不得删除断言或测试。完成后停留在本地，不执行 `git push`。

### 实际实现过程

1. 记录本阶段开始时 `tools/test_lab2.c` 和 `tools/grade.sh` 的 SHA-256，作为未修改基线。
2. 新建 `buddy_pmm.c` 和 `buddy_pmm.h`，保留独立的 `buddy_pmm_manager`，没有复用 Best-Fit 冒充伙伴系统。
3. 为 0 至 31 阶分别建立空闲链表。初始化可用区间时，根据绝对物理页号对齐条件选择当前可放入的最大 2 次幂块，因此能处理非对齐起点和非 2 次幂总长度。
4. 分配请求通过安全的逐级左移向上取整。若目标阶为空，则从更高阶取块并逐级拆分；分配块首的 `Page.property` 保存实际块大小。
5. 释放配对约定为：释放参数 `n` 按与分配相同的规则向上取整，结果必须等于块首记录的实际大小。伙伴页号使用 `PPN XOR block_size` 计算，只合并处于空闲状态且大小相同的伙伴。
6. `buddy_check()` 会检查各阶块大小、物理对齐、范围、Reserved 状态、可合并伙伴是否残留、空闲页计数，并实际执行 1 页和 3 页请求的分配与释放。
7. 在 Makefile 中加入 `PMM ?= best_fit`。仅当执行 `make PMM=buddy` 时定义 `USE_BUDDY_PMM`；未知模式直接报错。
8. `pmm.c` 通过条件编译选择管理器，默认仍为 `best_fit_pmm_manager`。
9. 本阶段没有创建独立测试套件，也没有修改老师的测试文件或评分脚本。

### 实际构建与验证

默认模式编译：

```bash
make clean
make -j2
```

结果：编译成功，新增的 `buddy_pmm.c` 同时通过编译。

Buddy 模式：

```bash
make clean
make PMM=buddy -j2
timeout --signal=TERM 10s make PMM=buddy qemu
```

关键输出：

```text
memory management: buddy_pmm_manager
check_alloc_page() succeeded!
```

这表明 Buddy 管理器已被选择，且其实际 `check()` 执行通过。

恢复默认 Best-Fit 后执行：

```bash
make clean
make grade
```

结果：

```text
-check page init:                          OK
-check best fit:                           OK
Total Score: 20/20
```

本阶段实现与切换过程一次编译通过，没有通过削弱断言或修改测试规避问题。最终结果保留在本地，未推送远程。
## Prompt 3：Buddy System 测试函数设计

### [PROMPT]

**任务：** 为当前 Lab 2 工程已经实现的 Buddy System 伙伴分配器设计并实现独立测试函数，验证分配、拆分、释放、合并、空闲块管理与边界条件是否正确。

**操作要求：**

1. 阅读真实的 `buddy_pmm.c`、`buddy_pmm.h`、`pmm.c`、`pmm.h` 和 `memlayout.h`。
2. 根据实际实现设计测试，不凭空假设内部函数或结构。
3. 在 Buddy 模式下接入测试，保证测试能够通过 `pmm_manager->check()` 或其他明确的测试入口实际执行。
4. 不修改既有 Best-Fit、First-Fit 测试函数。
5. 不通过弱化断言、删除失败用例或直接打印 PASS 的方式完成。
6. 将测试代码、真实输出及失败修复记录追加到实验报告，单独记录本份 Prompt。

### [RELY]

现有物理页分配接口：

```c
struct Page *alloc_pages(size_t n);
void free_pages(struct Page *base, size_t n);
size_t nr_free_pages(void);
```

Buddy System 对不同大小请求采用 2 的整数次幂块管理；其分配和释放方式应以当前实现约定为准。现有框架提供 `assert()`、`struct Page` 和页面状态相关宏。

### [GUARANTEE]

提供可以在真实内核中运行的 Buddy System 测试入口：

```c
void buddy_check(void);
```

该入口必须被实际执行；发生错误时由断言暴露失败，全部通过后输出 `Buddy System tests: PASS`。

### [SPECIFICATION]

测试覆盖以下情况：

1. 分配和释放 1、2、4、8 页，检查合法性、互不重叠和计数恢复；
2. 分配 3、5、7 页，检查分别向上取整为 4、8、8 页；
3. 验证高阶块逐级拆分且无页面丢失或重复分配；
4. 验证单侧释放、同阶伙伴合并、多级递归合并，以及相邻但非伙伴的块不被误合并；
5. 验证内存不足时返回 `NULL`，并保持计数和管理结构不变；
6. 验证起始地址不按最大块对齐、总页数非 2 的整数次幂的初始区域；
7. 多轮分配和回收后验证空闲链表、空闲页计数和页面状态一致，并恢复测试前状态。

测试必须使用安全、可恢复的独立区域，不得破坏内核原有页面状态。完成后实际编译并运行 Buddy 测试，再切回默认 Best-Fit 执行 `make grade`，确保仍为 20/20。不得修改 `tools/test_lab2.c`、`tools/grade.sh`，不得提交或推送。

### 实际实现过程

阅读 Buddy 分配器的各阶链表、块头 `property` 和释放配对约定后，将 `buddy_check()` 扩展为七组断言测试。测试先从真实管理器申请 32 页连续区域，再暂存并隔离原有各阶空闲链表，只在该区域内重建测试状态；测试结束后恢复原链表并归还整块页面。

针对拆分和合并测试，直接检查测试区域中的块头、阶数和链表成员关系，从而区分“确实按伙伴规则合并”和“碰巧能够再次分配”。非规则区域测试使用偏移 1 页、长度 29 页的子区间，并保留两侧保护页。

### 实际验证

Buddy 模式编译和 QEMU 测试均通过，七类测试逐项输出 PASS，最终输出：

```text
Buddy System tests: PASS
check_alloc_page() succeeded!
```

恢复默认 Best-Fit 后，`make grade` 输出 `Total Score: 20/20`。老师提供的测试文件哈希保持不变，本阶段未提交、未推送。

