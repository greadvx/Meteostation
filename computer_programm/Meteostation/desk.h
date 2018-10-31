#ifndef DESK_H
#define DESK_H

#include <QWidget>
#include <QVector>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QTime>
#include <QDebug>

namespace Ui {
class Desk;
}

class Desk : public QWidget
{
    Q_OBJECT
    bool stop = false;                          //условие окончания опроса устройства
    QSerialPort *serial;                        //порт
    QVector<double> plotterTemp;                //вектор для отрисовки данных по температуре
    QVector<double> plotterHum;                 //по влажности
    QVector<double> plotterBar;                 //по давлению
    QVector<double> plotterTime;                //вектор для отрисовки данных по времени
    int size = 20;                              //размер по оси Х
    int i = 0;                                  //время
    int attempt = 0;

public:
    explicit Desk(QWidget *parent = 0);
    ~Desk();

private slots:
   //для выбора порта
    void on_tempRadioButton_clicked();          //слот для показа графика температуры
    void on_humidRadioButton_clicked();         //слот для показа графика влажности
    void on_pressRadioButton_clicked();         //слот для показа графика давления

    void on_comboBoxPorts_activated(const QString &arg1);

    void on_startResumeButton_clicked();

    void on_stopButton_clicked();

private:
    Ui::Desk *ui;
    void setProtocol(const QString &);          //установка параметров протокола
    void addToContainers(QString, QString, QString);//функция добавления в контейнер
    void pauseTimer();                          //таймер паузы
    void updateLCD(QString, QString, QString);  //обновление экрана
    QString readBytesFromPort();                //считывание данных с МК
    void writeSignal();                         //запись управляющего сигнала
    void sizeChecker();                         //проверка размера

    void addTempGraph();                        //отрисовка графика температуры
    void addHumGraph();                         //отрисовка графика влажности
    void addPressGraph();                       //отрисовка графика давления
    void updateGraphs();                        //функция обновления графиков

    bool temp = false;                          //булевы переменные для работы с графиками
    bool hum = false;
    bool press = false;

    bool pushed = true;
};

#endif // DESK_H
