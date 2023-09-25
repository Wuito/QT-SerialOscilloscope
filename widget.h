#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <qDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QFileDialog>

#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QValueAxis>
#include <QTextStream>
#include <QTimer>
#include <vector>
#include <fstream>
#include <ctime>


#define MAX_X   90      //画图坐标轴限制
#define MAX_Y   180     //画图坐标轴限制
#define MODE    0       //默认1

typedef struct  AngleData       //传感器数据结构体
{
    float Sensor_1;
    float Sensor_2;
    float Sensor_3;
    float Sensor_4;
    float Sensor_5;
}Angle;

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QSplineSeries *lineSeries;
    QSplineSeries *lineSeries2;
    QSplineSeries *lineSeries3;
    QSplineSeries *lineSeries4;
    QSplineSeries *lineSeries5;
    QTimer *timerDrawLine;
    QChart *chart;
    QTimer *timerserial;


private slots:
    void Read_Date();       //读取串口数据
    #if MODE
    #else
    void serial_timerstart();
    #endif
    void str_to_num(char string[]);
    void find_port();           //查找可用串口
    void sleep(int msec);

    void on_send_button_clicked();
    void on_open_port_clicked();
    void on_close_port_clicked();
    void on_clear_button_1_clicked();
    void on_clear_button2_clicked();
    void on_send_modl_clicked();
    void on_receive_modl_clicked();
    void SaveRecvDataFile();
    void Chart_Init();
    void DrawLine();
    void updateData();

private:
    Ui::Widget *ui;
    QSerialPort *serialport;
    bool textstate_receive;
    bool textstate_send;
    QVector<QString> m_data;
    int count = 0;
};


#endif // WIDGET_H
