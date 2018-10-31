#include "desk.h"
#include "ui_desk.h"

Desk::Desk(QWidget *parent) :                                  //конструктор
    QWidget(parent),
    ui(new Ui::Desk)
{
    ui->setupUi(this);                                                          //инициализация пользовательского интерфейса
    this->setFixedSize(684,439);
    this->setWindowTitle("Meteostation");
    QStringList list;

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())      //цикл для вывода доступных серийных портов
    {   if (serialPortInfo.portName()[0] != 't')
            list.append(serialPortInfo.portName());
    }
    ui->comboBoxPorts->addItems(list);                                               //добавляем данные в контейнер
    ui->lcdPressure->setDigitCount(6);                                            //устанавливаем размер lcd box
    ui->lcdHumidity->setDigitCount(6);
    ui->lcdTemperature->setDigitCount(6);

    serial = new QSerialPort(this);

    ui->widget->addGraph();
    ui->widget->yAxis->setLabel(" ");
    ui->widget->xAxis->setLabel("Time, sec");

    QPalette colorPalette(palette());
    colorPalette.setColor(QPalette::Background, Qt::gray);
    this->setPalette(colorPalette);
 }
Desk::~Desk()
{
    delete ui;
    serial->close();
}


void Desk::on_tempRadioButton_clicked()                                         //слот на нажатие кнопки
{
    pushed = true;                                                              //температура
    ui->widget->yAxis->setRange(0, 40);                                         //устанавливаем предел по оси У
    this->temp = true;                                                          //выставляем значения переменных
    this->hum = false;
    this->press = false;
    updateGraphs();
}

void Desk::on_humidRadioButton_clicked()                                         //слот на нажатие кнопки
{                                                                                //влажность
    pushed = true;
    ui->widget->yAxis->setRange(0, 100);                                        //устанавливаем предел по оси У
    this->hum = true;                                                           //выставляем значения переменных
    this->temp = false;
    this->press = false;
    updateGraphs();
}

void Desk::on_pressRadioButton_clicked()
{
    pushed = true;
    ui->widget->yAxis->setRange(690, 800);
    this->press = true;
    this->hum = false;
    this->temp = false;
    updateGraphs();
}
void Desk::setProtocol(const QString &name)
{
    serial->setPortName(name);                                          //устанавливаем имя порта
    serial->setBaudRate(QSerialPort::Baud9600);                         //устанавливаем скорость передачи данных
    serial->setDataBits(QSerialPort::Data8);                            //устанавливаем тип входящих данных
    serial->setParity(QSerialPort::NoParity);                           //устанавливаем парность сигнала
    serial->setStopBits(QSerialPort::OneStop);                           //количество стоп-битов 1
    serial->setFlowControl(QSerialPort::NoFlowControl);                  //устанавливаем контоль потока
}
void Desk::addToContainers(QString tempStr, QString humStr, QString pressStr)
{                                                                       //функция добавления в контейнер
    bool ok = true;
    plotterTemp.append(tempStr.toFloat(&ok));
    plotterHum.append(humStr.toFloat(&ok));
    plotterBar.append(pressStr.toFloat(&ok));
}
void Desk::pauseTimer()
{
    QTime waitTime= QTime::currentTime().addSecs(1);                     //устанавливаем задержку
       while (QTime::currentTime() < waitTime && !stop)
           QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
void Desk::updateLCD(QString tempStr, QString humStr, QString pressStr)
{
    ui->lcdTemperature->display(tempStr);                                    //обновляем параметры
    ui->lcdHumidity->display(humStr);                                   //LCD экранов
    ui->lcdPressure->display(pressStr);
}
QString Desk::readBytesFromPort()
{
    QString buffer;
    QByteArray arr;
    arr = serial->readAll();                                              //считываем ответ
    buffer = QString::fromStdString(arr.toStdString());
    buffer.resize(37);
    return buffer;
}
void Desk::writeSignal()
{
    if (stop) return;
    serial->write("1");                                                    //отправляем в контроллер уникальный код
    serial->waitForBytesWritten();                                         //ожидаем отправку
    serial->waitForReadyRead();                                            //ожидаем доступ к считыванию
}
void Desk::sizeChecker()
{
    if (plotterTime.size() == size)
        size *= 2;
}
void Desk::addTempGraph()                                                   //отрисовка графика температуры
{
    ui->widget->xAxis->setRange(0,size);
    ui->widget->yAxis->setLabel("Temperature, C");
    ui->widget->graph(0)->setData(plotterTime, plotterTemp);
    ui->widget->replot();
    ui->widget->update();
    sizeChecker();
}
void Desk::addHumGraph()                                                   //отрисовка графика влажности
{
    ui->widget->xAxis->setRange(0,size);
    ui->widget->yAxis->setLabel("Humidity, %");
    ui->widget->graph(0)->setData(plotterTime, plotterHum);
    ui->widget->replot();
    ui->widget->update();
    sizeChecker();
}
void Desk::addPressGraph()                                                 //отрисовка графика давления
{
    ui->widget->xAxis->setRange(0, size);
    ui->widget->yAxis->setLabel("Pressure, mm r/s");
    ui->widget->graph(0)->setData(plotterTime, plotterBar);
    ui->widget->replot();
    ui->widget->update();
    sizeChecker();
}
void Desk::updateGraphs()
{
    if (this->temp) addTempGraph();                                     //обновляем показания графиков
    if (this->hum) addHumGraph();
    if (this->press)addPressGraph();
}

void Desk::on_comboBoxPorts_activated(const QString &arg1)
{
    if (arg1 == "cu.Bluetooth-Incoming-Port")
       {
           QMessageBox::information(this, "Attention", "There is nothing connected by Bluetooth.");
           return;
       }
}


void Desk::on_startResumeButton_clicked()
{
    qDebug() << ui->comboBoxPorts->currentText();
    setProtocol(ui->comboBoxPorts->currentText());
    serial->open(QIODevice::ReadWrite);                                 //открываем выбранный серийный порт
    stop = false;

    do{
        if (stop || !serial->isOpen()) break;

        writeSignal();                                                          //отправляем сигнал

        if (attempt > 0) {
            QString buffer = readBytesFromPort();
            QString tempStr, humStr, pressStr;
            qDebug() << buffer;
            tempStr = buffer.mid(7,2);                                              //выделяем показания температуры
            humStr = buffer.mid(16, 2);                                             //показания влажности
            pressStr = buffer.mid(25,6);                                            //показания давления
            if (serial->isOpen())
                addToContainers(tempStr, humStr, pressStr);                             //записываем информацию в контейнеры
            else break;
            plotterTime.append(i++);                                                //увеличиваем счетчик времени
            updateLCD(tempStr, humStr, pressStr);                                   //обновляем показания экранов
            if (this->pushed)
                updateGraphs();
            pauseTimer();                                                            //пауза
        } else {
            pauseTimer();
       }
        attempt++;
    }while(true);
}

void Desk::on_stopButton_clicked()
{
   stop = true;
   attempt = 0;
   serial->close();
}
