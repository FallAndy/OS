# Lab 4 验收与评分兼容性记录

本目录保存 Lab 4 的真实评分和启动记录。老师提供的 `tools/grade.sh`、
`tools/inject.py`、`tools/test_lab4.c` 及其断言、分值均未修改。

## 当前环境

- 当前默认 QEMU：`/home/fall/opt/qemu-4.1.1/bin/qemu-system-riscv64`，版本 4.1.1。
- 默认固件：`opensbi-riscv64-virt-fw_jump.bin`，启动横幅版本 OpenSBI v0.4。
- 原 QEMU 7.0.0 仍保留在 `/opt/qemu/bin/qemu-system-riscv64`，没有被覆盖。
- 当前终端可以直接执行 `make qemu` 和 `make grade`；也可以通过 Makefile 的
  `QEMU` 变量显式选择版本。

## QEMU 7 环境下的历史原始评分结果

执行：

```bash
cd /home/fall/oslab/OS/code
sh verification/lab4/reproduce_original_grade.sh
```

该脚本在临时副本中执行未经修改的 `make grade`，避免清理当前工作区的
`bin/`、`obj/`。`original_make_grade.log` 保存的是切换参考环境前，明确使用
QEMU 7.0.0/OpenSBI 1.0 得到的历史结果：

```text
Total Score: 0/30
```

原始评分使用 generic loader：

```text
-device loader,file=bin/ucore.img,addr=0x80200000
```

在 QEMU 7.0.0/OpenSBI 1.0 下，loader 会把镜像字节放入内存，但不会把
OpenSBI FW_DYNAMIC 的下一阶段入口设置为 `0x80200000`，启动输出为：

```text
Domain0 Next Address : 0x0000000000000000
```

因此失败发生在内核第一条指令执行之前，三项测试标志均不会出现。
`loader_boot.log` 保存原始 loader 启动输出；
`loader_force_raw_boot.log` 证明增加 `force-raw=on` 仍不会改变该结果。

## QEMU 7 环境下的兼容评分结果

执行：

```bash
cd /home/fall/oslab/OS/code
sh verification/lab4/reproduce_compatible_grade.sh
```

兼容脚本创建临时工程，运行官方 `inject.py`，复制官方 `grade.sh` 后只把
QEMU 启动参数替换为：

```text
-kernel bin/ucore.img
```

测试注入、断言、超时和计分规则保持不变。真实结果保存在
`compatible_make_grade.log`，串口输出保存在
`compatible_qemu_serial.log`：

```text
check PMM:             OK
check Process Create:  OK
check Schedule:        OK
Total Score: 30/30
```

这是兼容启动方式下的官方测试结果，不是原始 `make grade` 的结果。

## QEMU 4.1.1 参考环境下的最终原始评分结果

QEMU 4.1.1 独立安装于：

```text
/home/fall/opt/qemu-4.1.1
```

其默认 OpenSBI v0.4 使用 `fw_jump`，按参考环境约定跳转到
`0x80200000`。原始 generic loader 已把 `bin/ucore.img` 加载到该地址，
因此无需修改 `grade.sh`、`inject.py`、`test_lab4.c` 或计分规则。

使用显式路径执行原始评分：

```bash
cd /home/fall/oslab/OS/code
make QEMU="$HOME/opt/qemu-4.1.1/bin/qemu-system-riscv64" grade
```

当前终端的默认 QEMU 已指向 4.1.1，因此也可以直接执行：

```bash
make grade
```

真实结果为：

```text
check PMM:             OK
check Process Create:  OK
check Schedule:        OK
Total Score: 30/30
```

这是未经修改的原始 `make grade` 结果，与前述兼容脚本结果严格区分。终端
截图保存在 `report/images/lab4_original_grade_pass.png`。

## 正常启动

`normal_qemu.log` 使用当前 Makefile 的 `-kernel bin/ucore.img` 方式生成，
其中包含 `initproc` 的三行预期输出。随后出现的 `process exit!!.` panic
来自老师框架中尚未扩展的 `do_exit()`，与指导书给出的 Lab 4 运行结果一致。

控制台输入已将 OpenSBI legacy console 的负返回值归一化为“无输入”的 0，
因此 Kernel Debug Monitor 会稳定停留在单个 `K>` 等待输入，不再高速刷屏。
