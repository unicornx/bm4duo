看门狗实验说明：

本实验设置看门狗上电/复位后第一次定时器超时为 5 秒，后继定时器超时为 10秒。并在定时器超时中断处理函数中喂狗。

主程序每隔 1 秒打印一次 "---> 1s delay"， 所以正常我们看到如下打印结果：

```shell
Jump to monitor at 0x80000000.
OPENSBI: next_addr=0x80200020 arg1=0x80080000
====> test start
====> Running WatchDog example ......
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> Watchdog timer expired!
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> Watchdog timer expired!
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> Watchdog timer expired!
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> Watchdog timer expired!
---> 1s delay
---> 1s delay
......
```

尝试取消喂狗，即注释掉 `wdt_isr()` 中调用 `wdt_feeddog()`。因为我们设置为超时两次才重启，所以可以看到如下打印结果, 系统上电后先经过第一次五秒超时，因为不清中断，所以无限循环打印 `---> Watchdog timer expired!`, 再次超时 10 秒后系统重启：

```shell
OPENSBI: next_addr=0x80200020 arg1=0x80080000
====> test start
====> Running WatchDog example ......
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> 1s delay
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
......
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
---> Watchdog timer expired!
C.SCS/0/0.WD.URPL.SDI/25000000/6000000.BS/SD.PS.SD/0x0/0x1000/0x1000/0.PE.BS.SD/0x1000/0x8400/0x8400/0.BE.J.
FSBL Jb2829:g426d1e7-dirty:2024-05-15T16:17:56+08:00
st_on_reason=8090003
st_off_reason=0
P2S/0x1000/0xc00a400.
SD/0x9400/0x1000/0x1000/0.P2E.
DPS/0xa400/0x2000.
SD/0xa400/0x2000/0x2000/0.DPE.
cv181x DDR init.
ddr_param[0]=0x78075562.
pkg_type=5
D1_3_2
DDR3-2G-QFN
Data rate=1866.
DDR BIST PASS
PLLS.
PLLE.
C2S/0xc400/0x8fe00000/0x13200.
[R2TE:. 0/0x13200/0x13200/0.RSC.
 1M.S3/103x913f66]0P0r/e0 xs8y0s0t0e0m0 0i0n/i0tx fdeo0n0e.

RT: [1.320225]CVIRTOS Build Date:Apr 29 2024  (Time :11:00:29) 
RT: [1.326139]Post system init done
RT: [1.329462]dump_print_enable & log will not print
SD/0x1f600/0xfe00/0xfe00/0.ME.
L2/0x2f400.
SD/0x2f400/0x200/0x200/0.L2/0x414d3342/0xcafe2e9a/0x80200000/0x37600/0x37600
COMP/1.
SD/0x2f400/0x37600/0x37600/0.DCP/0x80200020/0x1000000/0x81900020/0x37600/1.
DCP/0x7442d/0.
Loader_2nd loaded.
Use internal 32k
Jump to monitor at 0x80000000.
OPENSBI: next_addr=0x80200020 arg1=0x80080000
====> test start
......
```
