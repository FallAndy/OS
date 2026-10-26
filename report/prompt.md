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

## Prompt 4：GDB 调试 QEMU 的 MMU 地址转换过程

### [PROMPT]

**任务：** 根据本地 Lab 2 实验指导书中的 `lab2_4_qemu_mmu_debug.html`，完成 QEMU MMU 地址转换的源码分析与真实 GDB 调试，观察一次 RISC-V Sv39 虚拟地址到物理地址的转换过程。

**操作要求：**

- 先检查当前 QEMU 版本、源码位置、调试符号与 GDB 环境，不直接照搬指导书中针对 QEMU 4.1.1 的旧命令。
- 当前环境使用 WSL2 Ubuntu 22.04，已有 RISC-V 工具链和 QEMU，优先复用现有环境。
- 若必须重新编译调试版 QEMU，应使用独立构建目录，不覆盖已安装的 QEMU，不执行 `sudo make install`。
- 使用宿主机 GDB 调试 QEMU 自身，使用 RISC-V GDB 调试被模拟的 ucore 内核，明确区分二者。
- 不破坏已通过测试的 Lab 2 内核和分配器代码。
- 必须执行真实调试，不能仅根据源码推断调试结果。

### [RELY]

读取并核对：

- `docs/course-guide/lab2/lab2_4_qemu_mmu_debug.html`
- `code/Makefile`
- `code/kern/init/entry.S`
- `code/kern/init/init.c`
- `code/kern/mm/mmu.h`
- `code/kern/mm/pmm.c`

在 QEMU 实际源码中定位 RISC-V MMU 地址翻译、页表遍历及 TLB 相关函数，不预先假设函数名和具体调用路径。

### [GUARANTEE]

交付一次可以复现的 MMU 调试实验，包含实际 QEMU 版本和调试环境、宿主与客户机 GDB 命令、真实断点命中、一个具体虚拟地址的转换、Sv39 调用路径、TLB 未命中处理和软件模拟行为，并保存必要的调试输出图。

### [SPECIFICATION]

1. 宿主 GDB 能对 QEMU 源码设置有效断点，客户机 GDB 能观察 ucore。
2. 选择真实执行地址，记录虚拟地址、页表基址、页表层级、PTE 和最终物理地址；大页映射需要解释提前终止原因。
3. 定位 TLB 查询和未命中实现，解释 QEMU 软件 TLB 与真实硬件 TLB 的逻辑关系和差异。
4. 将命令、源码路径、单步过程、输出图和结论写入 `report/report.md`，将 Prompt 和问题写入 `report/prompt.md`；未完成项必须明确说明。
5. 不自动提交或推送 GitHub。

### 实际调试过程

1. 检查得到 QEMU 版本为 7.0.0，源码位于 `/home/fall/qemu-7.0.0`，实际二进制为 `/opt/qemu/bin/qemu-system-riscv64`，对应构建产物为 `/home/fall/qemu-7.0.0/build/qemu-system-riscv64`。
2. `file` 和 `readelf` 证实该二进制未剥离且包含 `.debug_info`、`.debug_line`、`.symtab`，因此未重新编译或安装 QEMU。
3. 环境没有普通宿主 GDB，且 `sudo` 需要密码。为避免修改系统，将 Ubuntu GDB 12.1 和必要运行库解压到 `/tmp/oslab-gdb-local2`，通过 `LD_LIBRARY_PATH` 运行。
4. 使用 RISC-V GDB 连接 QEMU GDB stub，在 `kern_init=0xffffffffc02000d6` 处命中，读取 `satp=0x8000000000080205` 和根页表第 511 项。
5. 使用宿主 GDB 调试 QEMU，在 `riscv_cpu_tlb_fill`、`get_physical_address`、`get_page_addr_code_hostp`、`tlb_set_page` 设置条件断点，真实记录页表遍历和 TLB 状态。
6. 将三组真实输出整理为等宽排版图保存到 `report/images`。图片仅重排真实输出，报告中明确说明其性质。

### 遇到的问题与处理

1. **宿主 GDB 缺失：** 未使用需要密码的 `sudo apt install`，改为用户态下载并解压软件包；第一次下载被截断，校验失败后在新临时目录重新下载完整包。
2. **非登录 Shell 找不到交叉 GDB：** 改用 `/home/fall/riscv-elf-toolchains/bin/riscv64-unknown-elf-gdb` 绝对路径。
3. **`$pc` 被外层 Shell 处理：** 后续批处理使用已由符号表确认的具体地址，交互复现时仍可直接使用 `x/8i $pc`。
4. **现有 QEMU 带调试信息但经过优化：** `until` 可能跨出内联或优化后的函数，局部变量显示 `optimized out`。改为预先在 QEMU 7.0.0 的具体源码行设置条件断点，并对最终物理地址使用单一断点复核。
5. **批处理断点标签顺延：** 两个被优化合并的源码行没有单独命中，因此不采用顺延标签作为结论，而以断点实际所在行和最后一次单断点输出为准。

### 实际结果

对 `kern_init` 指令地址 `0xffffffffc02000d6` 的真实调试得到：

- `satp = 0x8000000000080205`，Sv39 根页表物理地址为 `0x80205000`；
- `VPN2=511`，根表 PTE 地址为 `0x80205ff8`；
- PTE 为 `0x200000cf`，PPN 为 `0x80000`，是带 `V/R/W/X/A/D` 的二级叶子项；
- 该叶子映射 1 GiB 大页，因此不继续访问 VPN1、VPN0 对应的下级页表；
- 最终物理地址为 `0x802000d6`；
- 软件 TLB 首次条目为无效值 `0xffffffffffffffff`，填充后同一地址再次查询时条目为 `0xffffffffc0200000`；
- `tlb_set_page` 实测写入 `VA page 0xffffffffc0200000 -> PA page 0x80200000`，权限值为 7，QEMU TCG 条目大小为 4 KiB。

本阶段没有修改内核和分配器源代码，未提交、未推送。
## Prompt 5：SLUB 内存对象分配器实现

### [PROMPT]

**任务：** 在现有 RISC-V ucore Lab 2 工程中，实现一个简化的 SLUB 内存对象分配器，以现有物理页面分配器为底层，实现不同大小内存对象的高效分配和释放。

**操作要求：**

1. 在当前 `lab2` 分支修改真实源码，新建 `code/kern/mm/slub.c` 和 `slub.h`。
2. 采用两层架构：底层使用 `alloc_pages()`、`free_pages()`，上层在物理页内管理小对象。
3. SLUB 保持为独立模块，不替换 `struct pmm_manager`。
4. 默认仍使用 Best-Fit，保留 Buddy System 实现与测试。
5. 通过明确编译开关增加可选初始化入口，默认不影响 `make grade`。
6. 本阶段完成实现、编译和基础启动；独立功能测试套件留给下一份 Prompt。
7. 不修改老师测试和评分脚本，追加报告，不提交、不推送。

### [RELY]

实际读取并遵循：

- `code/kern/mm/pmm.c`、`pmm.h`、`memlayout.h`、`mmu.h`
- `code/kern/mm/best_fit_pmm.c`
- `code/kern/mm/buddy_pmm.c`、`buddy_pmm.h`
- `code/kern/init/init.c`
- `code/libs/list.h`、`defs.h`
- `code/Makefile`

使用现有接口：

```c
struct Page *alloc_pages(size_t n);
void free_pages(struct Page *base, size_t n);
size_t nr_free_pages(void);
uintptr_t page2pa(struct Page *page);
```

物理页存储地址必须通过 `page2pa(page) + va_pa_offset` 转为内核虚拟地址，不能把 `struct Page *` 当作对象存储，也不能依赖被注释掉的 `KADDR()`。

### [GUARANTEE]

新增公开接口：

```c
void slub_init(void);
void *slub_alloc(size_t size);
void slub_free(void *ptr);
```

实现不能依赖动态堆分配器；在 `pmm_init()` 后通过可选开关初始化。

### [SPECIFICATION]

- 管理 8、16、32、64、128、256、512、1024、2048 字节分类。
- 非标准请求选择最小可容纳分类。
- slab 满时从 PMM 获取新页面。
- 大于 2048 字节的请求走连续页直接分配并保存释放元数据。
- `size == 0` 或溢出返回 `NULL`。
- 对象至少按指针大小对齐，不重叠、不覆盖元数据。
- `slub_free(NULL)` 安全返回。
- 普通对象可复用，slab 全空时页面归还 PMM，大对象释放全部连续页。
- 使用 `SLUB_CHALLENGE=1` 显式启用，默认关闭。
- 依次验证默认编译、SLUB 编译和启动、恢复默认 `make grade`。
- 不把尚未执行的完整功能测试标记为 PASS。

### 实际实现过程

实现采用页内元数据方案。每个小对象 slab 占用一个物理页，页首保存 magic、底层 `struct Page *`、所属 cache、链表节点、空闲对象链、对象计数和 512 位分配位图；对象区从按指针大小对齐的页首元数据之后开始。九个静态 cache 分别维护 partial 和 full slab 链表。

空闲对象的首个指针宽度空间用于保存下一空闲对象地址，已分配位图用于验证对象状态并阻止重复释放。slab 最后一个对象释放时，从 cache 链表删除并通过 `free_page()` 归还底层 PMM。

大对象在连续页首页保存 `slub_large` 元数据，记录底层页描述符、申请页数和请求大小，对外返回元数据之后的对齐地址。释放时将地址向下页对齐，读取 magic 并按记录调用 `free_pages()`。

新增 `SLUB_CHALLENGE ?= 0` 构建开关；值为 1 时定义 `SLUB_CHALLENGE`，由 `kern_init()` 在 `pmm_init()` 后调用 `slub_init()`。

### 实际验证

执行了：

```bash
make clean
make -j2
make clean
make SLUB_CHALLENGE=1 -j2
timeout --signal=TERM 12s make SLUB_CHALLENGE=1 qemu
make clean
make grade
```

SLUB 模式真实启动输出包含：

```text
memory management: best_fit_pmm_manager
check_alloc_page() succeeded!
SLUB: initialized 9 size classes (8..2048 bytes)
```

默认自动评分保持：

```text
-check page init:                          OK
-check best fit:                           OK
Total Score: 20/20
```

本阶段没有运行下一阶段的独立对象分配测试，因此只确认实现进入编译、初始化不变量通过且内核正常启动，不声称完整功能测试 PASS。未修改老师测试，未提交、未推送。
## Prompt 6：SLUB 分配器独立测试

### [PROMPT]

**任务：** 针对已实现的 SLUB 内存对象分配器，设计能够在 QEMU 中真实运行的独立测试，验证不同大小对象的分配、释放、复用和物理页回收。

**操作要求：**

1. 读取最新 `slub.c`、`slub.h`、`pmm.c`、`memlayout.h` 及相关代码，根据实际实现测试。
2. 不修改 First-Fit、Best-Fit、Buddy System 的既有测试。
3. 测试必须执行断言、地址和数据完整性检查，不能只打印 PASS。
4. 测试隔离并清理自身资源，不破坏 PMM 状态。
5. 测试后恢复默认 Best-Fit 并运行 `make grade`。
6. 追加 Prompt、设计、真实输出与错误修复过程，不提交、不推送。

### [RELY]

公开接口：

```c
void slub_init(void);
void *slub_alloc(size_t size);
void slub_free(void *ptr);
size_t nr_free_pages(void);
```

可以使用内核 `assert()`、`cprintf()` 和内存操作；页面大小及 slab 元数据必须来自当前真实实现。

### [GUARANTEE]

新增并实际执行：

```c
void slub_check(void);
```

通过可选 SLUB Challenge 模式接入内核，并为每组成功测试输出明确结果。

### [SPECIFICATION]

1. 分配 1、8、16、24、32、64、100、128、256、512、1024、2048 字节，检查地址、对齐和不重叠。
2. 写入、读取不同字节模式，验证对象间数据隔离。
3. 释放部分同类对象并重新申请，验证安全复用和活跃对象数据。
4. 动态读取单页对象容量并分配更多对象，强制扩展到第二个 slab。
5. 释放空 slab，验证页面归还且活跃 slab 不被误回收。
6. 测试 13、100、300、3000、4096、8192 字节请求和大对象释放。
7. 测试零长度、空指针、过大请求、多轮分配释放和开始/结束空闲页一致性。
8. QEMU 真实运行后恢复默认配置并保持 `make grade 20/20`。

预期成功输出必须由真实断言之后产生，不得提前打印。

### 实际测试实现

在 `slub.c` 内新增结构校验器，遍历九个 cache 的 partial/full 链表，检查 slab magic、page 到 KVA 映射、所属 cache、对象数量、free_count、分配位图和空闲链节点数量。partial slab 必须有空闲但不能全空，full slab 必须没有空闲节点。

七组测试各自记录 `nr_free_pages()`，完整释放本组对象后断言恢复。每组结束还要求所有 cache 链表为空。大对象虽不进入小对象 cache 链表，但通过首页元数据、请求大小、页数和最终 PMM 页数恢复进行验证。

内存不足测试没有耗尽真实内存，而是请求超过整个可管理物理页范围的连续对象；底层 PMM 安全返回 `NULL`，并断言空闲页和 SLUB 结构不变。

### 实际结果

执行：

```bash
make clean
make SLUB_CHALLENGE=1 -j2
timeout --signal=TERM 15s make SLUB_CHALLENGE=1 qemu
```

真实输出：

```text
SLUB test 1 - size classes: PASS
SLUB test 2 - data integrity: PASS
SLUB test 3 - object reuse: PASS
SLUB test 4 - slab expansion: PASS
SLUB test 5 - page reclaim: PASS
SLUB test 6 - irregular sizes: PASS
SLUB test 7 - state consistency: PASS
SLUB tests: PASS
```

测试代码第一次在 QEMU 中运行即通过全部断言，没有发生需要修改 SLUB 实现或弱化测试的运行错误。补丁应用期间曾因统一差异的行数声明错误被 Git 整体拒绝，修正补丁元数据后正常应用；这没有产生半写入源码，也不是分配器功能错误。

恢复默认配置后 `make grade` 仍输出 `Total Score: 20/20`。测试代码保留在 SLUB 模块中，未修改老师测试，未提交、未推送。
---

## Prompt 7：最终交付审查

### [PROMPT]

**任务：** 对当前 Lab 2 工程进行最终交付审查，阅读本地 `lab2_2_1_exercise.html`、`lab2_4_qemu_mmu_debug.html` 和 `lab2_5_labs_requirement.html`，逐项核对 `report/report.md`、`report/prompt.md` 与 `report/images/`。

**核查重点：**

1. 检查必做练习、思考题、GDB 分支任务和 SLUB Challenge；
2. 核查 First-Fit 四个核心函数、First-Fit/Best-Fit 改进空间、物理内存探测思考题；
3. 确认 Buddy 与 SLUB 各自具有实现和独立测试两份 Prompt；
4. 仅根据真实源码和测试记录补缺，不编造结果；
5. 移除 Lab 1 遗留、过期内容与错误路径，但保留真实实验成果；
6. 重新运行默认 Best-Fit `make grade`，并核验 Buddy、SLUB 测试证据；
7. 最后执行 `git status`、`git diff --check`，列出待提交文件与未完成事项；
8. 不修改已通过的分配器实现，不提交、不推送。

### 实际审查与修订过程

- 逐项对照三份本地指导书，确认原报告缺少 First-Fit 四函数说明、First-Fit/Best-Fit 政策改进思考、物理内存未知时的探测回答和实验整体知识总结；
- 依据真实 `default_pmm.c`、`best_fit_pmm.c`、`pmm.c`、`dtb.c` 补充分析，没有改动分配器源码；
- 确认 Prompt 2/3 分别对应 Buddy 实现与测试，Prompt 5/6 分别对应 SLUB 实现与测试；
- 用哈希确认 Lab 2 报告前部和三张旧截图与 `lab1` 分支内容一致后，从当前 Lab 2 交付物中移除，原成果仍由 `lab1` 分支保存；
- 初次用非登录 shell 构建时因工具链不在 `PATH` 报错，改用 `bash -lic` 后完成真实回归；
- Buddy 七组 QEMU 测试通过，SLUB 七组 QEMU 测试通过；
- 最后恢复默认 Best-Fit，`make grade` 实际得到 `Total Score: 20/20`；
- 最终 Git 状态与差异检查结果以本次交付审查结论为准，未执行 commit 或 push。
