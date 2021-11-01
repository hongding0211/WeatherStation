# WeatherStation Neo

> 项目基于 [Miniature Weather Station - ESP8266](https://www.instructables.com/id/Minitaure-Weather-Station-ESP8266/)

对原项目做了大幅度的修改。替换了 API，新增设置页面及 OTA 升级功能；额外绘制了 PCB 板，精简了部分硬件及功能。

## 0. 关于
- 实时天气信息
- 未来 3 日天气预报

![](https://raw.githubusercontent.com/HongDing97/imgs/main/IMG_0346.JPEG)

> 原项目的 UI 动画也删掉了，感觉有点冗余

初次启动（更换新的 Wi-Fi 环境）会有配网指示。手机连接热点并访问 ```192.168.4.1``` 即可打开设置页面。

![](https://raw.githubusercontent.com/HongDing97/imgs/main/IMG_7012.JPEG)

在系统启动的 5 分钟内，会一直暴露热点供其他终端连接。

因此当需要访问设置页面时，可以断电重启设备，然后连接热点并访问 ```192.168.4.1```。

> 当然也可以直接通过分配的 IP 地址访问

在设置页面中可以自定义天气的定位，如果输入的定位有误或者解析失败，会退回使用 IP 所在的定位。

### 设置页面

> 页面适配深色模式

![](https://raw.githubusercontent.com/HongDing97/imgs/main/IMG_0311.JPEG)

## 1. 硬件

### 主要选型

- ESP8266-12F
- OLED 1.3寸
  - I2C
  - 单色
  - SSD1306 驱动
  > 注意针脚顺序，有的版本 VCC 在最左边
  > 
  > ![](https://raw.githubusercontent.com/HongDing97/imgs/main/20211101104504.png)
- AMS1117-3.3
- 10k 电阻
- 10uF 电容

**单个硬件成本约 70，将 PCB 和 3D 模型直接发送给工厂加工即可。**

> PCB 和硬件设计都比较业余，我也是跟着一个教程简单学了下，不过跑起来没问题。
>
> 没有选择贴片，这样焊接起来比较容易。

![](https://raw.githubusercontent.com/HongDing97/imgs/main/IMG_6999.JPEG)

原项目是带有室内温湿度检测功能的，我一开始做的版本也有。

但是后来我发现芯片本身散发出的热量会极大影响数值的准确性（可能和我把温湿度传感器直接粘在后背也有关系）。

目前我也没想到更好的解决方案，更重要的是我想尽可能地精简功能，让信息展示更加高效，所以就把这个模块给删了。

> 硬件电路上还是保留了 DHT 传感器的输入和供电。
>
> 想要加上去也可以，或者替换成其他的输入/输出。

**稳压模块的管脚和标注的顺序是反的，焊接的时候注意下**

> ![](https://raw.githubusercontent.com/HongDing97/imgs/main/IMG_70051.jpeg)

## 2. 软件

### 开发环境

- Clion
- Platform IO
- macOS(m1) / Windows 亲测都可行

第一次载入工程后，Re-Init一下

![](https://raw.githubusercontent.com/HongDing97/imgs/main/20211101113512.png)

### API

把下列 API 替换成自己申请的 Key：

 - OpenWeather - openweathermap.org

   ```c++
   // include/WeatherClient.h
   const String appID = "<APP ID>";
   ```

 - IP 定位 - amap.com

   ```c++
   // include/LocationClient.h
   const String apiKey = "<API KEY>";
   ```

	- IP 查询 - ipify.org

### 有线下载
> 需要 USB 转串口模块

第一次烧录可以使用有线的方式，之后更推荐 OTA 更新，更方便。

通过 USB 转串口模块连接 PCB 板下载电路。

下载时，先拉低 GPIO0 再上电。
### OTA 更新

访问 /update 页面上传固件，Clion 默认编译后路径：
```
.pio/build/esp12e/firmware.bin
```
