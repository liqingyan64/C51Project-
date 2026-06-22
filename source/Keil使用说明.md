# Keil C51 使用说明

## 1. 新建 Keil 工程

1. 打开 Keil uVision。
2. 新建工程，建议保存到：

```text
C:\Users\DELL\Documents\C51单片机课程设计\source
```

3. 芯片选择 `Atmel AT89C51`。
4. 将 `password_lock.c` 添加到工程。

## 2. 设置生成 HEX 文件

1. 打开 `Options for Target`。
2. 进入 `Output` 页面。
3. 勾选 `Create HEX File`。
4. 编译工程。

编译成功后，会生成类似文件：

```text
password_lock.hex
```

## 3. 在 Proteus 中加载程序

1. 双击 Proteus 中的 AT89C51。
2. 在 `Program File` 中选择 Keil 生成的 HEX 文件。
3. 将 `Clock Frequency` 设置为：

```text
11.0592MHz
```

4. 启动仿真。

## 4. 默认操作

默认密码：

```text
123456
```

按键功能：

```text
#：输入密码开锁
A：修改密码
B：查看开锁次数
C：输入时退格
*：输入时清空
D：输入管理员密码后清零开锁次数
```

如果 12864 屏幕完全不显示，优先检查：

```text
HDG12864L-4 的 VDD/GND 是否接好
P0.0-P0.7 是否接到 D0-D7
P0.0-P0.7 是否通过 RESPACK-8 上拉到 VCC
CS1/RES/A0/RW/E 是否分别接到 P2.3/P2.4/P2.0/P2.1/P2.2
AT89C51 的 EA 是否接 VCC
AT89C51 是否加载了 HEX 文件
```
