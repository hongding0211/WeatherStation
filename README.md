# WeatherStation Neo

> 项目基于 [Miniature Weather Station - ESP8266](https://www.instructables.com/id/Minitaure-Weather-Station-ESP8266/)
>
> 对原项目做了大幅度的修改。替换了所有 API，新增设置页面及 OTA 升级功能；额外绘制了 PCB 板，精简了部分硬件及功能。

## 0. 关于

### 外观

### 设置页面

![](https://raw.githubusercontent.com/HongDing97/imgs/main/IMG_0311.JPEG)

> 页面特意适配了深色模式

## 1. 硬件

### 主要选型

	- ESP8266-12F
	- OLED 1.3寸

**单个硬件成本约 70，将 PCB 和 3D 模型直接发送给工厂加工即可。**

> PCB 画的很简单也很业余，不过好处就是焊接起来非常简单。应该只要不手残，都能成功。

原项目是带有室内温湿度检测功能的，我一开始做的版本也有。但是后来我发现芯片本身散发出的热量会极大影响数值的准确性（可能和我把温湿度传感器直接粘在后背也有关系）。目前我也没想到更好的解决方案，更重要的是我想尽可能地精简功能，让信息展示更加高效，所以就把这个模块给删了。

> 硬件电路上还是保留了 DHT 传感器的输入和供电。想要加上去也可以，或者替换成其他的输入/输出。

外壳是项目原作者设计的，不过我做了一点点修改，让屏幕可以更加契合模型。

### 原理图

## 2. 软件

### 开发环境

- Clion
- Platform IO
- macOS(m1) / Windows 亲测都可行

### API

 - OpenWeather - openweathermap.org

   ```c++
   // include/WeatherClient.h
   const String appID = "<APP ID>";
   ```

 - IP 定位 - amap.com

   ```c++
   // include?LocationClient.h
   const String apiKey = "<API KEY>";
   ```

	- IP 查询 - ipify.org

### 有线下载

### OTA 更新

