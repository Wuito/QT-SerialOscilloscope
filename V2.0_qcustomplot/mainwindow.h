#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"

// 蓝牙部分
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/qbluetoothdevicediscoveryagent.h>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothSocket>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QBluetoothAddress>


// 串口部分
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QIODevice>

struct  plotData       //数据结构体
{
    bool state;
    double data1;
    double data2;
    double data3;
    double data4;
    double data5;
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void bluetoothDataSend(QString str);
    ~MainWindow();

    void find_port();

    std::vector<plotData> plot_buffer; //数据表
    plotData clash;  //用于数据计算，临时缓冲数据

public slots:
    void Plot_TimeData_Update();

private slots:

    //曲线可视化
    void on_checkBox_1_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);

    //串口槽函数
    void serial_timerstart();
    void Read_Date();

    void on_refresh_port_clicked();
    void on_open_port_clicked();
    void on_close_port_clicked();
    void on_recive_data_clicked();
    void on_clean_data_clicked();
    void on_savedata_csv_clicked();
private:
    Ui::MainWindow *ui;

    // 串口类
    QSerialPort *serialport;
    QTimer *timerserial;
    QByteArray serial_bufferClash;    //串口接收数据缓冲
    QStringList serial_BuferList;      //串口数据帧缓冲

    // 绘图控件的指针
    QCustomPlot *pPlot1;
    // 状态栏指针
    QStatusBar *sBar;
    // 绘图控件中曲线的指针
    QCPGraph *pGraph1_1;
    QCPGraph *pGraph1_2;

    void QPlot_init(QCustomPlot *customPlot);
    void Plot_Show_Update(QCustomPlot *customPlot, double n1, double n2);
    double cnt;
};
#endif // MAINWINDOW_H
