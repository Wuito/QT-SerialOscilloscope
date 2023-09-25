# QT-SerialOscilloscope
A basic serial assistant is designed using qt framework. It has the function of analyzing data, saving data and drawing real-time curve.

## 上位机使用教程

安装好qt之后使用Qt Creator打开SerialPortDate.pro工程文件之后，配置好工程环境，点击运行按钮。

电脑插上蓝牙无线串口、或者其它COM设备，下位机打开后，选中目标设备的串口号，打开串口接收数据，完成后点击停止接收数据，再次采集只需要再点击打开串口。注意不要点击`文本接收`和`文本发送`切换为HEX模式后无法和下位机通信。HEX模式只用于开发中的测试，模板中的所有数据都按照字符串格式传输和解析。

下位机通信协议测试：单片机的串口通信，使用9600波特率，8位数据位，1位停止位，0校验。发送数据帧格式：

```
*aaa,bbb,ccc,eee,fff#
```

其中帧头帧尾都是字符型`*`和`#`

字母表示数字，数字最大位数不超过float类型最大值（2的32次方），

上位机的二次开发可以参考源代码注释。



##上位机源码结构重点

上位机的所有文件操作通过对全局变量`m_data`的操作进行，从串口接收数据时就把数据写入到`m_data`中，保存时就把数据从`m_data`中以此写入到txt文件里。

画图部分依赖`Data`结构体，给结构体赋值后使用`DrawLine()`函数刷新图像数据，每调用一次`DrawLine()`就会在画图区域新增一组数据。



## 通信协议

单片机编程伪代码：

```\
typedef struct sonsor
{
    float sensor1;
    float sensor2;
    float sensor3;
    float sensor4;
}	sonsor;

sonsor cap;

int main(void)
{
    Sys_Init();
    sonsor_Init();
    while(1)
    {
        delay_ms(100);
        cap.sensor1 = sonsor_Read(CHANNEL_1);
        cap.sensor2 = sonsor_Read(CHANNEL_2);
        cap.sensor3 = sonsor_Read(CHANNEL_3);
        cap.sensor4 = sonsor_Read(CHANNEL_4);
        
        //printf() 需要串口重定向到uart输出
        printf("*%0.1f,%0.1f,%0.1f,%0.1f,0#");	//第五位没采集到数据暂时设置为0
        
    }
}
```
