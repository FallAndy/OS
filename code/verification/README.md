# Lab 3 测试证据

本目录保存 Lab 3 的真实构建、启动和评分日志。官方测试源码、断言和计分规则未被修改。

## 文件说明

- `build.log`：当前源码执行 `make -j2` 的完整终端记录。
- `qemu_boot.log`：执行 `timeout 6s make qemu` 的启动记录。
- `shutdown_after_1.log`：使用测试值 `SHUTDOWN_AFTER=1` 验证自动关机路径的真实记录；1 不是老师规定值。
- `original_make_grade.log`：原始 `make grade` 终端记录，结果为 `0/100`。
- `original/`：原始评分方式产生的三份 QEMU 串口日志。
- `compatible_make_grade.log`：兼容启动方式执行同一评分逻辑的终端记录，结果为 `100/100`。
- `compatible/`：兼容启动方式产生的三份 QEMU 串口日志。
- `grade_qemu7_compat.sh`：隔离测试树中的兼容脚本副本。它与官方 `tools/grade.sh` 的测试断言和计分逻辑相同，仅将 QEMU 启动参数从 generic loader 改为 `-kernel bin/ucore.img`。

## 两种评分方式

原始评分命令：

```bash
cd /home/fall/oslab/OS/code
make grade
```

原始脚本使用：

```bash
-device loader,file=bin/ucore.img,addr=0x80200000
```

当前 QEMU 7.0.0/OpenSBI 1.0 环境下，镜像被写入 `0x80200000`，但 OpenSBI 显示 `Domain0 Next Address = 0x0`，没有进入内核，因此三个用例均无法观察到测试标志，总分为 `0/100`。

兼容评分在隔离副本中运行。执行步骤为：复制工程、运行官方 `tools/inject.py`、复制官方 `grade.sh` 为 `grade.compat.sh`，仅把启动参数替换为：

```bash
-kernel bin/ucore.img
```

随后执行未改变断言和计分规则的 `grade.compat.sh`。OpenSBI 得到 `Domain0 Next Address = 0x80200000`，三项测试分别得到 35、25、40 分，总分为 `100/100`。

## 结果解释

`100/100` 是兼容启动方式下同一套官方测试的结果，不是原始 `make grade` 的结果。仓库中的 `tools/grade.sh` 和 `tools/inject.py` 均未因兼容验证而修改。

## 复现命令

普通构建与启动：

```bash
cd /home/fall/oslab/OS/code
make -j2
timeout 6s make qemu
```

原始评分：

```bash
cd /home/fall/oslab/OS/code
make grade
```

兼容评分：

```bash
cd /home/fall/oslab/OS/code
sh verification/reproduce_compatible_grade.sh
```

兼容脚本使用 `mktemp` 创建隔离工程，不修改当前工作树。测试结束后会输出隔离目录路径，其中保留本次运行的三份原始串口日志。

自动关机机制验证：

```bash
cd /home/fall/oslab/OS/code
make clean
make SHUTDOWN_AFTER=1 qemu
```

该命令仅验证配置入口和 `sbi_shutdown()` 路径：第一次输出 `100 ticks` 后 QEMU 正常退出。测试值 1 不代表老师规定的关机次数；正式使用时必须替换为老师确认的正整数。
