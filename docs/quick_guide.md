**快速上手**

<!-- TOC -->

- [搭建环境](#搭建环境)
- [构建](#构建)
- [烧录和测试](#烧录和测试)

<!-- /TOC -->

# 1. 搭建环境

我们的开发 host 机为 x86_64，推荐 Ubuntu 20 以上版本系统。

下载本仓库，假设下载后的路径为 `$WS/bm4duo`。

```shell
git clone git@github.com:unicornx/bm4duo.git
```

下载 Sophgo 提供的编译工具链，假设下载后的路径为 `$WS/host-tools`。

```shell
git clone git@github.com:sophgo/host-tools.git
```

记得修改 `$WS/bm4duo/Makefile` 中的 `TOOL_PATH`，使其指向下载的 Sophgo 编译工具链。因为我们采用交叉构建方式编译，所以选择针对 RISC-V 裸机交叉开发的 GCC：riscv64-elf-x86_64, 举例如下。**记得将 `$WS` 替换为您本机的实际工作路径**：

```makefile
TOOL_PATH :=  $WS/host-tools/gcc/riscv64-elf-x86_64/bin
```

# 2. 构建

进入 `$WS/bm4duo`，执行 `build.sh` 脚本，注意该文件是一个符号链接，实际执行的文件是 bm4duo 仓库下的 `./scripts/build.sh`。

直接运行 `build.sh` 会打印简单的使用帮助，该脚本比较简单，感兴趣也可以直接阅读脚本内容了解功能：

```shell
$ ./build.sh 
Usage: [BOARD=board] build.sh '-h'|'-a'|<case>
  board: duo/duo256, default is duo256
  -h: print this usage
  -a: try to build all cases, just used for integrity test and no output created
  <case>: build <case>, <case> can be: gpio uart pwm rtc rtc_irq wdt adc i2c timer spi 
```

需要注意的是：

- 本仓库支持的 duo 产品包括 duo 和 duo256，默认构建 duo256 的程序，如果要切换到 duo，可以利用环境变量 `BOARD` 进行指定。
- `-a` 仅用于完整性构建测试，即批处理一次性构建所有的用例。
- 正常情况下，一次构建只针对一个用例。譬如我们要构建针对 duo256 的 gpio 这个测试用例，可以输入：
  ```shell
  ./build.sh gpio
  ```

构建完成后生成的程序文件在 bm4duo 仓库下的 `release_out` 目录下，文件名称为 `fip.bin`


# 3. 烧录和测试

生成 `fip.bin` 文件后可以将其写入 sd-card 的第一个分区中，然后将 sd-card 插入 duo 开发板的 micro-sd 卡槽中，上电即可工作。可以连接串口观察程序输出。

为方便在 Linux 环境下烧写 sd-card，bm4duo 仓库根目录下提供了一个 mkcard.sh 的脚本，如果您使用的是市面上常用的 USB 接口的 sdcard 读写器，可以直接运行该脚本将生成的 fip.bin 文件 写入 sd-card 的第一个分区。该脚本也是一个符号链接，实际执行的文件是 bm4duo 仓库下的 `./scripts/flash_sdcard.sh`。使用该脚本时有以下几点注意一下：
- 该脚本默认您的读卡器连上 Linux 主机后枚举的设备文件名为 `/dev/sdb1`。如果你的设备枚举名称不同，可以在执行脚本时通过环境变量 `DEV` 自行指定。譬如 `DEV=/dev/sdc1 ./mkcard.sh`。
- 该脚本将 `DEV` 设备挂载到默认的挂载点 `UDISK`, 如果您使用不同的挂载点路径，可以直接修改 `mkcard.sh` 脚本中的环境变量 `UDISK`, 或者和 `DEV` 类似在执行脚本命令时指定。譬如 `UDISK=/mnt/u-disk ./mkcard.sh`
- 执行该脚本需要 sudo 权限。

有关 sd-card 的制作可以参考《将 RVOS 移植到 MilkV-Duo 上》的 “sd-card 启动盘的制作” 章节介绍。

有关开发板接串口可以参考《将 RVOS 移植到 MilkV-Duo 上》的 “其他注意事项：串口连接和设置” 章节介绍。
