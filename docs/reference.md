**参考资料和文档汇总**

**注意：这里列出的参考文档处于持续更新中，所以这里只给出了文档的官方链接，建议对参考文档保持持续跟踪。**

# Duo

总入口：这里比较值得看的是 Duo 256 的管脚图
- <https://milkv.io/docs/duo/getting-started/duo>
- 中文版：<https://milkv.io/zh/docs/duo/getting-started/duo>

Duo 的硬件资料：<https://github.com/milkv-duo/duo-files/tree/main/duo>。

建议经常查阅的资料手册包括：

- 【DUO-PINOUT】[duo 开发板引脚定义图][duo-gpio-pinout]
- 【DUO-SCHEMA】duo 开发板电路原理图，目前有 [v1.1][duo-schema-v1.1] 和 [v1.2][duo-schema-v1.2] 两个版本，对应具体硬件版本。如果不特别指明，默认为 1.2。
- 【CV180X-TRM】[CV1800B/CV1801B Preliminary Datasheet Version: 0.3.0.0][cv180x-trm]: Duo 使用的 SoC 是 CV1800b。技术参考手册 (中文版)。
- 【CV180X-PIN】[CV180xb 引脚定义表 V1][cv180x-pin]
- 【CV180X-INT】[CV180x 中断定义表 V1][cv180x-interrupt]
- 【CV180X-CLK】[CV180x 时钟定义表 V1][cv180x-clock]

# Duo 256

总入口：这里比较值得看的是 Duo 256 的管脚图
- <https://milkv.io/docs/duo/getting-started/duo256m>
- 中文版：<https://milkv.io/zh/docs/duo/getting-started/duo256m>

Duo 256 的硬件资料：<https://github.com/milkv-duo/duo-files/tree/main/duo-256M>。

建议经常查阅的资料手册包括：

- 【DUO256-PINOUT】[duo 256M 开发板引脚定义图][duo256-gpio-pinout]
- 【DUO256-SCHEMA】[duo 256M 开发板电路原理图 v1][duo256-schema-v1]
- 【SG2002-TRM】[SG2002 技术参考手册][sg2002-trm]: Duo 256 使用的 SoC 是 SG2002。需要注意的是，这里的 SoC datasheet 不确保是最新的，官方的 SOC TRM 可以访问 <https://github.com/sophgo/sophgo-doc/releases>。这里是官方定期发布 pdf 形式的地方。我们可以下载下载最新版本的中文版本技术参考手册：`sg2002_trm_cn.pdf`。如果你喜欢尝试最新的 TRM，也可以根据发布的源码自行构建，具体参考 <https://github.com/sophgo/sophgo-doc/blob/main/SG200X/TRM/doc/design.md> 中的 “编译方法” 章节的介绍。


[duo-schema-v1.1]:https://github.com/milkv-duo/duo-files/blob/main/duo/hardware/duo-schematic-v1.1.pdf
[duo-schema-v1.1]:https://github.com/milkv-duo/duo-files/blob/main/duo/hardware/duo-schematic-v1.2.pdf
[cv180x-trm]:https://github.com/milkv-duo/duo-files/blob/main/duo/datasheet/CV1800B-CV1801B-Preliminary-Datasheet-full-zh.pdf
[cv180x-pin]:https://github.com/milkv-duo/duo-files/blob/main/duo/datasheet/CV180xb-Pinout-v1.xlsx
[cv180x-interrupt]:https://github.com/milkv-duo/duo-files/blob/main/duo/datasheet/CV180X-Interrupt-v1.xlsx
[cv180x-clock]:https://github.com/milkv-duo/duo-files/blob/main/duo/datasheet/CV180X-Clock-v1.xlsx
[duo256-schema-v1]:https://github.com/milkv-duo/duo-files/blob/main/duo-256M/hardware/duo-256m-v1.0-schematic.pdf
[sg2002-trm]:https://github.com/milkv-duo/duo-files/tree/main/duo-256M/datasheet
[duo256-gpio-pinout]:https://milkv.io/docs/duo/getting-started/duo256m#gpio-pinout
[duo-gpio-pinout]:https://milkv.io/docs/duo/getting-started/duo#gpio-pinout