#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 给widget绘图控件，设置个别名，方便书写
    pPlot1 = ui->widget_1;

    // 状态栏指针
    sBar = statusBar();

    // 初始化图表1
    QPlot_init(pPlot1);
    cnt = 0;

    setWindowTitle("数据采集系统");
    serialport = new QSerialPort;
    find_port();                    //查找可用串口
    timerserial = new QTimer();
    QObject::connect(serialport,&QSerialPort::readyRead, this, &MainWindow::serial_timerstart);
    QObject::connect(timerserial,SIGNAL(timeout()), this, SLOT(Read_Date()));


    ui->close_port->setEnabled(false);//设置控件不可用
}


// 析构函数
MainWindow::~MainWindow()
{
    delete ui;
}

//查找串口
void MainWindow::find_port()
{
    //查找可用的串口
    bool fondcom = false;
    ui->com->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);   //设置串口
        if(serial.open(QIODevice::ReadWrite))
        {
            //
            ui->com->addItem(serial.portName());        //显示串口name
            fondcom = true;
            QString std = serial.portName();
            QByteArray comname = std.toLatin1();
            //QMessageBox::information(this,tr("SerialFond"),tr((const char *)comname.data()),QMessageBox::Ok);
            serial.close();
            ui->open_port->setEnabled(true);
        }
    }
    if(fondcom==false)
    {
        QMessageBox::information(this,tr("Error"),tr("Serial Not Fond!Plase cheak Hardware port!"),QMessageBox::Ok);
    }
}

/* 打开并设置串口参数 */
void MainWindow::on_open_port_clicked()
{
    update();
    //find_port();     //重新查找com
     //初始化串口
         serialport->setPortName(ui->com->currentText());        //设置串口名
         if(serialport->open(QIODevice::ReadWrite))              //打开串口成功
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
             serialport->setFlowControl(QSerialPort::NoFlowControl);     //设置流控制
             // 设置控件可否使用
            ui->close_port->setEnabled(true);
            ui->open_port->setEnabled(false);
            ui->refresh_port->setEnabled(false);
         }
         else    //打开失败提示
         {
            // Sleep(100);

             QMessageBox::information(this,tr("Erro"),tr("Open the failure"),QMessageBox::Ok);
         }
}


/* 关闭串口并禁用关联功能 */
void MainWindow::on_close_port_clicked()
{
    serialport->clear();        //清空缓存区
    serialport->close();        //关闭串口

    ui->open_port->setEnabled(true);
    ui->close_port->setEnabled(false);
    ui->refresh_port->setEnabled(true);
}

/* 开始接收数据
 * */
void MainWindow::on_recive_data_clicked()
{
    QString str = "START_SEND_DATA\r\n";
    QByteArray str_utf8 = str.toUtf8();
    if(serialport->isOpen())serialport->write(str_utf8);
    else QMessageBox::information(this,tr("ERROE"),tr("串口未连接，请先检查串口连接"),QMessageBox::Ok);
}

void MainWindow::serial_timerstart()
{
    timerserial->start(1);
    serial_bufferClash.append(serialport->readAll());
}

//串口接收数据帧格式为：帧头'*' 帧尾'#' 数字间间隔符号',' 符号全为英文格式
void MainWindow::Read_Date()
{
    QString string;
    QStringList serialBuferList;
    int list_length = 0;//帧长
    QString str = ui->Receive_text_window->toPlainText();
    timerserial->stop();//停止定时器
//    qDebug()<< "[Serial LOG]serial read data:" <<serial_bufferClash;

    QByteArray bufferbegin = "*";   //帧头
    int index=0;
    QByteArray bufferend = "#";     //帧尾
    int indexend = 1;
    QByteArray buffercashe;
    index = serial_bufferClash.indexOf(bufferbegin,index);
    indexend = serial_bufferClash.indexOf(bufferend,indexend);
//    qDebug()<< index<< indexend;
    int bufferlens=0;
    if((index<serial_bufferClash.size())&&(indexend<serial_bufferClash.size()))
    {
        bufferlens = indexend - index-1;
        buffercashe = serial_bufferClash.mid(index+1,bufferlens);
        qDebug()<< "[Serial LOG]serial chack data:" <<buffercashe;
        string.prepend(buffercashe);
        serialBuferList = string.split(" ");      //数据分割
        list_length=serialBuferList.count();    //帧长
        if (list_length>1)
        {
            clash.data1 = serialBuferList[0].toDouble();
            clash.data2 = serialBuferList[1].toDouble();
            plot_buffer.push_back(clash);
            clash.data1 = serialBuferList[2].toDouble();
            clash.data2 = serialBuferList[3].toDouble();
            plot_buffer.push_back(clash);
            clash.data1 = serialBuferList[4].toDouble();
            clash.data2 = serialBuferList[5].toDouble();
            plot_buffer.push_back(clash);
        }
    }
    else
    {
        qDebug()<< "[Serial LOG][ERROR]recive data:" <<serial_bufferClash;
    }
    str+="succeed:"+buffercashe;
    str += "  ";
    ui->Receive_text_window->clear();
    ui->Receive_text_window->append(str);
    serial_bufferClash.clear();
}

/*  刷新串口按键的按钮槽函数
 * */
void MainWindow::on_refresh_port_clicked()
{
    find_port();
}



// 绘图图表初始化
void MainWindow::QPlot_init(QCustomPlot *customPlot)
{

    // 创建定时器，用于定时生成曲线坐标点数据
    QTimer *timer = new QTimer(this);
    timer->start(10);
    connect(timer, SIGNAL(timeout()), this, SLOT(Plot_TimeData_Update()));

    // 图表添加两条曲线
    pGraph1_1 = customPlot->addGraph();
    pGraph1_2 = customPlot->addGraph();

    // 设置曲线颜色
    pGraph1_1->setPen(QPen(Qt::red));
    pGraph1_2->setPen(QPen(Qt::black));

    // 设置坐标轴名称
    customPlot->xAxis->setLabel("X-Times");
    customPlot->yAxis->setLabel("Channel Data");

    // 设置y坐标轴显示范围
    customPlot->yAxis->setRange(-2, 2);

    // 显示图表的图例
    customPlot->legend->setVisible(true);
    // 添加曲线名称
    pGraph1_1->setName("Channel1");
    pGraph1_2->setName("Channel2");

    // 设置波形曲线的复选框字体颜色
    ui->checkBox_1->setStyleSheet("QCheckBox{color:rgb(255,0,0)}"); // 设定前景颜色,就是字体颜色
    // 允许用户用鼠标拖动轴范围，用鼠标滚轮缩放，点击选择图形:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

int data_lens = 0;
// 定时器溢出处理槽函数。用来生成曲线的坐标数据。
void MainWindow::Plot_TimeData_Update()
{

    int lens = plot_buffer.size();
    if (lens > data_lens)
    {
        for(int i=data_lens;i<lens;i++)
        {
            Plot_Show_Update(pPlot1, plot_buffer[i].data1, plot_buffer[i].data2);
            data_lens++;
            qDebug()<<"[Plot LOG]data_lens:"<<data_lens<< "size:"<<lens;
        }
    }
}

// 曲线更新绘图
void MainWindow::Plot_Show_Update(QCustomPlot *customPlot, double n1, double n2)
{
    cnt++;
    // 给曲线添加数据
    pGraph1_1->addData(cnt, n1);
    pGraph1_2->addData(cnt, n2);

    // 设置x坐标轴显示范围，使其自适应缩放x轴
    customPlot->xAxis->setRange( 0, (pGraph1_1->dataCount() > 100) ? (pGraph1_1->dataCount()) : 100);
    // 更新绘图，这种方式在高填充下太浪费资源。rpQueuedReplot，可避免重复绘图。
    customPlot->replot(QCustomPlot::rpQueuedReplot);

    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0; // 开始到现在的时间，单位秒
    //计算帧数
    static double lastFpsKey;
    static int frameCount;
    frameCount++;
    if (key - lastFpsKey > 1) // 每1秒求一次平均值
    {
        // 帧数和数据总数
        ui->statusbar->showMessage(
            QString("Refresh rate: %1 FPS, Total data volume: %2")
                .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
                .arg(customPlot->graph(0)->data()->size() + customPlot->graph(1)->data()->size()),
            0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

/* 清空缓存数据
 * */
void MainWindow::on_clean_data_clicked()
{
    qDebug()<< "[Clean Data]";
    plot_buffer.clear();
    data_lens = 0;
    cnt = 0;
    pGraph1_1->data().data()->clear();
    pGraph1_2->data().data()->clear();
    pPlot1->graph(0)->data().clear();
    pPlot1->graph(1)->data().clear();
}

// setVisible设置可见性属性，隐藏曲线，不会对图例有任何影响。推荐使用。
void MainWindow::on_checkBox_1_stateChanged(int arg1)
{
    if (arg1)
    {
        pGraph1_1->setVisible(true);
    }
    else
    {
        pGraph1_1->setVisible(false); // void QCPLayerable::setVisible(bool on)
    }
    pPlot1->replot();
}

void MainWindow::on_checkBox_2_stateChanged(int arg1)
{
    if (arg1)
    {
        pGraph1_2->setVisible(true);
    }
    else
    {
        pGraph1_2->setVisible(false); // void QCPLayerable::setVisible(bool on)
    }
    pPlot1->replot();
}

// 保存缓冲区数据为csv文件
void MainWindow::on_savedata_csv_clicked()
{
     if(plot_buffer.size()<1)
     {
         QMessageBox::information(this, "提示","当前数据为空");
         return;
     }
     serialport->clear();        //清空缓存区
     timerserial->stop();
     serialport->close();        //关闭串口
     ui->open_port->setEnabled(true);
     ui->close_port->setEnabled(false);
     QString csvFile = QFileDialog::getExistingDirectory(this);
     QDateTime current_date_time =QDateTime::currentDateTime();
     QString current_date =current_date_time.toString("yyyy_MM_dd_hh_mm");
     csvFile += tr("/sensor_Save_%1.csv").arg(current_date);
     if(csvFile.isEmpty())
     {
        QMessageBox::information(this,tr("警告"),tr("文件路径错误，无法打开文件，请重试"),QMessageBox::Ok);
     }
     else
     {
         qDebug()<< csvFile;
         QFile file(csvFile);
         if ( file.exists())
         {
                 //如果文件存在执行的操作，此处为空，因为文件不可能存在
         }
         file.open( QIODevice::ReadWrite | QIODevice::Text );
         QTextStream out(&file);
         out<<tr("data1,")<<tr("data2,\n");     //写入表头
         // 创建 CSV 文件
         for (const auto &data : plot_buffer) {
             out << QString("%1,%2").arg(data.data1).arg(data.data2) << "\n";
         }
         file.close();
         QMessageBox::information(this, "提示","数据保存成功");
     }
     serialport->open(QIODevice::ReadWrite);        //打开串口
     ui->open_port->setEnabled(false);
     ui->close_port->setEnabled(true);
}
