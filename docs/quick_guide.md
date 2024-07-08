**快速上手**

<!-- TOC -->

- [1. 搭建环境](#1-搭建环境)
- [2. 构建](#2-构建)
- [3. 烧录和测试](#3-烧录和测试)
- [4. Jtag + GDB 调试](#4-jtag--gdb-调试)
	- [4.1. 硬件环境](#41-硬件环境)
	- [4.2. 软件环境](#42-软件环境)

<!-- /TOC -->

注：本文介绍的快速上手基于 Ubuntu/Linux 环境。

# 1. 搭建环境

我们的开发主机（host）为 x86_64，推荐采用 Ubuntu 20 或者以上版本系统。

```shell
$ lsb_release -a
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 20.04.6 LTS
Release:	20.04
Codename:	focal
```

APT 安装一些依赖的工具
```shell
$ sudo apt update
$ sudo apt install git build-essential python3
```

下载本仓库，假设你的工作路径为 `$WS`, 且下载本仓库的路径为 `$WS/bm4duo`。

```shell
$ cd $WS
$ git clone git@github.com:unicornx/bm4duo.git
```

下载 Sophgo 提供的编译工具链，假设下载后的路径为 `$WS/host-tools`。

```shell
$ cd $WS
$ git clone git@github.com:sophgo/host-tools.git
```

记得修改 `$WS/bm4duo/Makefile` 中的 `TOOL_PATH`，确保其指向下载的 Sophgo 编译工具链。因为我们采用交叉构建方式编译，所以选择针对 RISC-V 裸机交叉开发的 GCC：riscv64-elf-x86_64, 举例如下。

```makefile
TOOL_PATH :=  $WS/host-tools/gcc/riscv64-elf-x86_64/bin
```

新版本的 `$WS/bm4duo/Makefile` 中的 `TOOL_PATH` 默认值已经改为 `/opt/bm4duo-toolchain`。所以为了尽量不修改原始仓库代码，我们也可以在系统中建立一个 `/opt/bm4duo-toolchain` 的符号链接，将其指向 `$WS/host-tools/gcc/riscv64-elf-x86_64/bin`。

```shell
$ sudo ln -s $WS/host-tools/gcc/riscv64-elf-x86_64/bin /opt/bm4duo-toolchain
```

**记得将以上路径中的 `$WS` 替换为您本机的实际工作路径**。

# 2. 构建

进入 `$WS/bm4duo`，执行 `build.sh` 脚本，注意该文件是一个符号链接，实际执行的文件是 bm4duo 仓库下的 `./scripts/build.sh`。

直接运行 `build.sh` 会打印简单的使用帮助，该脚本比较简单，感兴趣也可以直接阅读脚本内容了解功能：

```shell
$ ./build.sh 
Usage: [BOARD=board] build.sh '-h'|'-a'|<case>
  board: duo/duo256, default is duo256
  -h: print this usage
  -a: try to build all cases, just used for integrity test and no output created
  <case>: build <case>, <case> can be: gpio uart pwm rtc rtc_irq wdt adc i2c timer spi dbg
```

需要注意的是：

- 本仓库支持多个 duo 产品，默认构建 duo256 的程序，如果要切换到其他产品，譬如 duo，可以利用环境变量 `BOARD` 进行指定, 譬如要构建 duo 的程序，可以执行 `BOARD=duo ./build.sh xxx`。
- `-a` 仅用于完整性构建测试，即批处理一次性构建所有的用例。
- 正常情况下，一次构建只针对一个用例。譬如我们要构建针对 duo256 的 gpio 这个测试用例，可以输入：
  ```shell
  ./build.sh gpio
  ```

构建完成后生成的程序文件在 bm4duo 仓库下的 `release_out` 目录下，文件名称为 `fip.bin`

# 3. 烧录和测试

生成 `fip.bin` 文件后可以将其写入 sd-card 的第一个分区中，然后将 sd-card 插入 duo 开发板的 micro-sd 卡槽中，上电即可工作。可以连接串口观察程序输出。

为方便在 Linux 环境下烧写 sd-card，bm4duo 仓库根目录下提供了一个名为 `mkcard.sh` 的脚本，如果您使用的是市面上常用的 USB 接口的 sdcard 读写器，可以直接运行该脚本将生成的 `fip.bin` 文件 写入 sd-card 的第一个分区。该脚本也是一个符号链接，实际执行的文件是 bm4duo 仓库下的 `./scripts/flash_sdcard.sh`。使用该脚本时有以下几点注意一下：

- 执行该脚本需要 sudo 权限。
- 该脚本默认您的读卡器连上 Linux 主机后枚举的设备文件名为 `/dev/sdb1`。如果你的设备枚举名称不同，可以在执行脚本时通过环境变量 `DEV` 自行指定。譬如假设您的 sdcard 设备的第一个分区枚举为 `/dev/sdc1`, 则我们可以在 Linux 命令行输入 `DEV=/dev/sdc1 ./mkcard.sh` 来烧写程序。
- 该脚本将设备 `$DEV` 挂载到挂载点 `$UDISK`, 如果您使用不同的挂载点路径，可以直接修改 `mkcard.sh` 脚本中的环境变量 `UDISK`, 或者和 `DEV` 类似在执行脚本命令时指定。譬如假设您的挂载点路径为 `/mnt/u-disk`, 则我们可以在 Linux 命令行输入 `UDISK=/mnt/u-disk ./mkcard.sh` 来烧写程序。

有关 Linux 上 sd-card 的制作可以参考[《将 RVOS 移植到 MilkV-Duo 上》][1] 的 “sd-card 启动盘的制作” 章节介绍。其他系统（如 Windows 上操作可自行尝试，这里不再给出。）

有关 Linux 上开发板接串口可以参考[《将 RVOS 移植到 MilkV-Duo 上》][1] 的 “其他注意事项：串口连接和设置” 章节介绍。其他系统（如 Windows 上操作可自行尝试，这里不再给出。）

# 4. Jtag + GDB 调试

## 4.1. 硬件环境

调试时，我们需要将调试主机和开发板之间通过特殊的 CKLINK 进行连接，连接示意图如下，具体可以参考 [T-Head DebugServer 用户手册][2]。

![](./diagrams/Debug-Env.png)

**图 1：调试环境**

**注意** 目前我们发现由于 Duo 开发板的限制，采用平头哥默认的官方 CKLink 反而连接不上，但是我们发现可以采用 Sipeed 开发的一款 四合一功能的设备 SLogic Combo 8 进行连接。具体有关产品的介绍可以访问 [SLogic Combo 8 的官网][3]。具体购买可以上淘宝。

![](./diagrams/SLogic-Combo-8.png)

**图 2：SLogic Combo 8**

SLogic Combo 8 是一款兼有逻辑分析仪、CKLink Debugger、DAP-Link Debugger、USB2UART 功能的开发工具。我们这里只用到 CKLink Debugger 的功能。

关于如何将开发板和 Host 主机通过 CKLink 进行连接，具体参考 [SLogic Combo 8 作为 CKLink 使用说明][4]。

按下切换按键，确保指示灯切换为黄色。

以 duo 256 为例，具体连线时，参考 [duo 256 引脚图][5]。将 CKLink 的 GND/TCK/TDI/TDO/TMS 和 duo 256 的 GND(3)/GP3(5)/GP0(1)/GP1(2)/GP2(4) 分别相连。

```
CKlink        Duo256M
  GND <------> GND(3)
  TCK <------> GP3(5)
  TDI <------> GP0(1)
  TDO <------> GP1(2)
  TMS <------> GP2(4)
```

## 4.2. 软件环境

软件环境特指 **图 1：调试环境** 中 Host 上的软件布署。目前有两种方式：

- **方式一**：采用 Windows 上 CDS 集成开发环境，这是一个具有 GUI 的开发环境，具体介绍请阅读 [《基于 CDS 集成开发环境进行调试的说明》](./develop-windows.md)。其实基于 CDS 环境除了调试，也可以进行开发，有关基于 CDS 开发的内容，这里不做介绍，感兴趣可以自己研究。

- **方式二**：采用 Ubuntu/Linux 环境，继续延续了本文前面部分 Linux 下命令行操作的习惯，具体介绍请阅读 [《基于 Ubuntu/Linux 进行调试的说明》](./develop-linux.md)。


[1]:https://zhuanlan.zhihu.com/p/691697875
[2]:https://www.xrvm.cn/document?temp=introduction-2&slug=t-head-debug-server-user-manual
[3]:https://wiki.sipeed.com/hardware/zh/logic_analyzer/combo8/index.html
[4]:https://wiki.sipeed.com/hardware/zh/logic_analyzer/combo8/use_cklink_function.html
[5]:https://milkv.io/docs/duo/getting-started/duo256m#gpio-pinout
[6]:https://blog.csdn.net/winter99/article/details/117464598

