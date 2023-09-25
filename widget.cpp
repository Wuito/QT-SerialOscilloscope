#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <iostream>

Angle Data;
QByteArray buffer;

//int data1_sensor={};
//data2_sensor={};
//data3_sensor={};
//data4_sensor={};
//data5_sensor={};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    /****************************************************************
     * 数据与窗体结构初始化
    ****************************************************************/
    ui->setupUi(this);
    ui->send_modl->setText("文本发送");
    textstate_send = true;              //发送文本框的文本显示模式，true为文本格式发送/false为HEX格式发送
    ui->receive_modl->setText("文本接收");
    textstate_receive = true;           //接收文本框的文本显示模式，true为文本模式/false为HEX模式
    ui->channel1->setChecked(true);ui->channel2->setChecked(true);ui->channel3->setChecked(true);
    ui->channel4->setChecked(true);ui->channel5->setChecked(true);      //通道复选框
    Data.Sensor_1 = (float)MAX_Y/2;
    Data.Sensor_2 = (float)MAX_Y/2;
    Data.Sensor_3 = (float)MAX_Y/2;
    Data.Sensor_4 = (float)MAX_Y/2;
    Data.Sensor_5 = (float)MAX_Y/2;

    setWindowTitle("数据采集系统");
    serialport = new QSerialPort;



    find_port();                    //查找可用串口
    connect(ui->save_data, SIGNAL(clicked()), this, SLOT(SaveRecvDataFile()));      // 保存接收文本框中的数据
    Chart_Init();
    /*定时器*/
    timerDrawLine = new QTimer();
    connect(timerDrawLine, SIGNAL(timeout()), this, SLOT(DrawLine()));      //定时器绑定画图槽函数

    #if MODE    //数据接收模式

    #else
    timerserial = new QTimer();
    QObject::connect(serialport,&QSerialPort::readyRead, this, &Widget::serial_timerstart);
    QObject::connect(timerserial,SIGNAL(timeout()), this, SLOT(Read_Date()));
    #endif

    ui->send_button->setEnabled(false);     //设置控件不可用
    ui->close_port->setEnabled(false);
    ui->save_data->setEnabled(false);
}

Widget::~Widget()
{
    delete ui, serialport;
}

//打开串口
void Widget::on_open_port_clicked()
{
   update();
   sleep(100);      //延时100ms
   find_port();     //重新查找com
    //初始化串口
        serialport->setPortName(ui->com->currentText());        //设置串口名
        if(serialport->open(QIODevice::ReadWrite))
        {
            serialport->setBaudRate(ui->baud->currentText().toInt());       //设置波特率
            switch(ui->bit->currentIndex())                   //设置数据位数
            {
                case 8:serialport->setDataBits(QSerialPort::Data8);break;
                default: break;
            }
            switch(ui->jiaoyan->currentIndex())                   //设置奇偶校验
            {
                case 0: serialport->setParity(QSerialPort::NoParity);break;
                default: break;
            }
            switch(ui->stopbit->currentIndex())                     //设置停止位
            {
                case 1: serialport->setStopBits(QSerialPort::OneStop);break;
                case 2: serialport->setStopBits(QSerialPort::TwoStop);break;
                default: break;
            }
            serialport->setFlowControl(QSerialPort::NoFlowControl);
            // 设置控件可否使用
            timerDrawLine->start(100);
            ui->send_button->setEnabled(true);
            ui->close_port->setEnabled(true);
            ui->save_data->setEnabled(true);
            ui->open_port->setEnabled(false);
        }
        else    //打开失败提示
        {
            sleep(100);

            QMessageBox::information(this,tr("Erro"),tr("Open the failure"),QMessageBox::Ok);
        }
}


//关闭串口
void Widget::on_close_port_clicked()
{
    serialport->clear();        //清空缓存区
    serialport->close();        //关闭串口
    timerDrawLine->stop();      //关闭波形刷新

    ui->send_button->setEnabled(false);
    ui->open_port->setEnabled(true);
    ui->close_port->setEnabled(false);
}

//发送数据，在本软件中要成功使用串口解析数据，则发送必须以文本格式直接发送
void Widget::on_send_button_clicked()
{
    if(textstate_send == true)  //文本模式
    {
        serialport->write(ui->send_text_window->toPlainText().toLatin1());
    }
    if(textstate_send == false)     //16进制
    {
        QString str = ui->send_text_window->toPlainText();
        if (str.contains(" ")) str.replace(QString(" "),QString("")); //替换空格
        QByteArray sendbuf = QByteArray::fromHex(str.toLatin1());
        ui->send_text_window->clear();
        ui->send_text_window->append(sendbuf);
        serialport->write(sendbuf);
    }
}


#if MODE


#else
//串口接收数据操作
//串口接收数据帧格式为：帧头'*' 帧尾'#' 数字间间隔符号',' 符号全为英文格式
void Widget::Read_Date()
{
    int bufferlens = 0;     //帧长
    QString str = ui->Receive_text_window->toPlainText();
    timerserial->stop();//停止定时器，

    qDebug()<<buffer;

    QByteArray bufferbegin = "*";   //帧头
    int index=0;
    QByteArray bufferend = "#";     //帧尾
    int indexend = 0;
    QByteArray buffercashe;

    index = buffer.indexOf(bufferbegin,index); //查找帧头
    indexend = buffer.indexOf(bufferend,indexend); //查找帧尾
    if((index<buffer.size())&&(indexend<buffer.size()))
    {
        bufferlens = indexend - index + 1; //计算帧长度
        buffercashe = buffer.mid(index,bufferlens); //截取出数据帧
    }

    char recvdata[buffercashe.size()];
    memset(recvdata,0,sizeof(recvdata));
    memcpy(recvdata,buffercashe.data(),bufferlens-1);
    recvdata[buffercashe.size()-1]=35;
    if(recvdata[0]=='*'&&recvdata[buffercashe.size()-1]=='#')   //二次帧检查
    {
        str_to_num(recvdata);       //更新数据并缓存到保存区
        str+="succeed:";    //在文本窗口给出提示
        str+=tr(buffercashe);
        str += "  ";
        ui->Receive_text_window->clear();
        ui->Receive_text_window->append(str);
    }
    else
    {
        str+="error! "; //错误处理
        str+=tr(buffercashe);
        str += "  ";
        ui->Receive_text_window->clear();
        ui->Receive_text_window->append(str);
    }
    buffer.clear();
}

void Widget::serial_timerstart()
{
    timerserial->start(4);
    buffer.append(serialport->readAll());
}
#endif


//解析数字
void Widget::str_to_num(char string[])
{
    int i=1,j=0;
    int num[5] ={0};
    std::string snum = "000";   //接收的数字位数最大三位
    for(int five=0;five<5;five++)
    {
        while(string[i]!=','&& string[i]!=35)  //解析单个数字位，测试用数据格式 *2,13,110,120,130#
        {
            snum[j]=string[i];
            i++;
            j++;
        }
        if(j==1) {snum[2]=snum[0];snum[0]='0';}
        if(j==2) {snum[2]=snum[1];snum[1]=snum[0];snum[0]='0';}   //根据数字位数进行顺序矫正
        num[five] = std::stoi(snum,0,10);
        i++;
        j=0;
    }
    if(ui->channel1->isChecked()) Data.Sensor_1 = (float)num[0];  //更新当前传感器数据组
    else Data.Sensor_1 = MAX_Y/2;
    if(ui->channel2->isChecked()) Data.Sensor_2 = (float)num[1];
    else Data.Sensor_2 = MAX_Y/2;
    if(ui->channel3->isChecked()) Data.Sensor_3 = (float)num[2];
    else Data.Sensor_3 = MAX_Y/2;
    if(ui->channel4->isChecked()) Data.Sensor_4 = (float)num[3];
    else Data.Sensor_4 = MAX_Y/2;
    if(ui->channel5->isChecked()) Data.Sensor_5 = (float)num[4];
    else Data.Sensor_5 = MAX_Y/2;
    updateData();
}

/*
    函   数：SaveRecvDataFile
    描   述：保存数据按钮点击槽函数
    输   入：无
    输   出：无
*/
void Widget::SaveRecvDataFile()
{
    if(m_data.size()<1)
    {
        QMessageBox::information(this, "提示","当前数据为空");
        return;
    }
    serialport->clear();        //清空缓存区
    serialport->close();        //关闭串口
    timerDrawLine->stop();      //关闭波形刷新
    ui->send_button->setEnabled(false);
    ui->open_port->setEnabled(true);
    ui->close_port->setEnabled(false);
    ui->save_data->setEnabled(false);

    QString csvFile = QFileDialog::getExistingDirectory(this);
    if(csvFile.isEmpty())
       return;
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("MM_dd_hh_mm");
    csvFile += tr("/%1.csv").arg(current_date);
    qDebug()<< csvFile;
    QFile file(csvFile);
    if ( file.exists())
    {
            //如果文件存在执行的操作，此处为空，因为文件不可能存在
    }
    file.open( QIODevice::ReadWrite | QIODevice::Text );
    QTextStream out(&file);
    out<<tr("Time,")<<tr("sensor1,")<<tr("sensor2,")<<tr("sensor3,")<<tr("sensor4,")<<tr("sensor5,\n");     //写入表头
    // 创建 CSV 文件
    for (const auto &data : m_data) {           //测试格式： *2,13,110,120,130#
        out << data << "\n";
    }
    file.close();
    QVector<QString>().swap(m_data);
    QMessageBox::information(this, "提示","数据保存成功");
}

//查找串口
void Widget::find_port()
{
    //查找可用的串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->com->clear();
            ui->com->addItem(serial.portName());
            serial.close();
        }
    }
}

//延时函数
void Widget::sleep( int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}


//曲线设置初始化
void Widget::Chart_Init()
{
    //初始化QChart的实例
    chart = new QChart();
    lineSeries = new QSplineSeries();
    //设置曲线的名称
    lineSeries->setName("曲线1");
    chart->addSeries(lineSeries);

    lineSeries2 = new QSplineSeries();
    lineSeries2->setName("曲线2");
    chart->addSeries(lineSeries2);
    lineSeries3 = new QSplineSeries();
    lineSeries3->setName("曲线3");
    chart->addSeries(lineSeries3);
    lineSeries4 = new QSplineSeries();
    lineSeries4->setName("曲线4");
    chart->addSeries(lineSeries4);
    lineSeries5 = new QSplineSeries();
    lineSeries5->setName("曲线5");
    chart->addSeries(lineSeries5);
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    //设置坐标轴显示的范围
    axisX->setMin(0);
    axisX->setMax(MAX_X);
    axisY->setMin(0);
    axisY->setMax(MAX_Y);
    //设置坐标轴上的格点
    axisX->setTickCount(10);
    axisY->setTickCount(10);
    //设置坐标轴显示的名称
    QFont font("Microsoft YaHei",8,QFont::Normal);//微软雅黑。字体大小8
    axisX->setTitleFont(font);
    axisY->setTitleFont(font);
    axisX->setTitleText("X-时间");
    axisY->setTitleText("Y-角度");
    //设置网格不显示
    axisY->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    //把曲线关联到坐标轴
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);
    lineSeries2->attachAxis(axisX);
    lineSeries2->attachAxis(axisY);
    lineSeries3->attachAxis(axisX);
    lineSeries3->attachAxis(axisY);
    lineSeries4->attachAxis(axisX);
    lineSeries4->attachAxis(axisY);
    lineSeries5->attachAxis(axisX);
    lineSeries5->attachAxis(axisY);
    //把chart显示到窗口上
    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);      // 设置渲染：抗锯齿，如果不设置那么曲线就显得不平滑
}

//更新曲线函数
void Widget::DrawLine()
{
    if(count > MAX_X)
    {
        //当曲线上最早的点超出X轴的范围时，剔除最早的点，
        lineSeries->removePoints(0,lineSeries->count() - MAX_X);
        chart->axisX()->setMin(count - MAX_X);
        chart->axisX()->setMax(count);
    }
    else{
        chart->axisX()->setMin(0);
        chart->axisX()->setMax(MAX_X);
    }
    lineSeries->append(count, (int)Data.Sensor_1);
    lineSeries2->append(count, (int)Data.Sensor_2);
    lineSeries3->append(count, (int)Data.Sensor_3);
    lineSeries4->append(count, (int)Data.Sensor_4);
    lineSeries5->append(count, (int)Data.Sensor_5);
    count ++;
}


void Widget::on_clear_button_1_clicked()    //清空发送文本框
{
    ui->send_text_window->clear();
}

void Widget::on_clear_button2_clicked()     //清空接收文本框
{
    ui->Receive_text_window->clear();
    QVector<QString>().swap(m_data);        //清空缓存区数据
    count = 0;              //恢复X轴初始状态
    lineSeries->clear();    //清空曲线
    lineSeries2->clear();
    lineSeries3->clear();
    lineSeries4->clear();
    lineSeries5->clear();
}

//接收框文本模式转换
void Widget::on_receive_modl_clicked()
{
    if(ui->receive_modl->text() == "文本接收")
    {
        textstate_receive = false;
        ui->receive_modl->setText("hex接收");
    }
    else
    {
        ui->receive_modl->setText("文本接收");
        textstate_receive = true;
    }
}

//发送框文本转换
void Widget::on_send_modl_clicked()
{
    if(ui->send_modl->text() == "文本发送")
    {
        textstate_send = false;
        ui->send_modl->setText("hex发送");
    }
    else
    {
        ui->send_modl->setText("文本发送");
        textstate_send = true;
    }
}

void Widget::updateData()
{
    // 获取当前系统时间
    QDateTime currentTime = QDateTime::currentDateTime();
    QString currentTimeString = currentTime.toString(Qt::ISODate);

    // 将数据保存到数据结构中
    QString dataString = QString("%1,%2,%3,%4,%5,%6")       //字符串处理函数 .arg对应前方的%1等占位符位置
        .arg(currentTimeString)
        .arg(Data.Sensor_1)
        .arg(Data.Sensor_2)
        .arg(Data.Sensor_3)
        .arg(Data.Sensor_4)
        .arg(Data.Sensor_5);
    //qDebug() << dataString;
    m_data.append(dataString);
    if(m_data.size()>5000)                                  //缓存区最大保存5000组数据，超出5000后会自动停止接收数据
    {
        serialport->clear();        //清空缓存区
        serialport->close();        //关闭串口
        timerDrawLine->stop();      //关闭波形刷新
        QMessageBox::information(this, "提示","数据量缓存过大，请保存数据再重新打开串口");
        SaveRecvDataFile();
    }
    //std::cout<< m_data.size()<< std::endl;
}
