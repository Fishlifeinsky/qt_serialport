#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QTextCursor>
#include <QTextDocument>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTimer>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    serialPort.setBaudRate(115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setParity(QSerialPort::NoParity);
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        ui->port->addItem(info.portName());
    }
    serialPort.setPortName(ui->port->currentText());    //初始化串口
    port = ui->port->currentText();

    repos = 0;

    ui->textEdit->installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);

    connect(&serialPort,&QSerialPort::readyRead,this,[&]{
        QString r = serialPort.readAll();
        processStr(r);
        qDebug()<<"接收了"<<r;
    });

    connect(&serialPort, &QSerialPort::errorOccurred, this, &Widget::handleError);

    fileFormat.clear();
    fileFormat = set.fileFormatList();
    tranText = set.tranTextColor();
    reText = set.reTextColor();

    connect(&set,&setWidget::setChange,this,[&]{
        fileFormat.clear();
        fileFormat = set.fileFormatList();
        tranText = set.tranTextColor();
        reText = set.reTextColor();
    });

    QTextCursor cursor = ui->textEdit->textCursor();
    QTextCharFormat cf;
    cf.setForeground(QColor(tranText));
    ui->textEdit->setTextCursor(cursor);
    set.hide();

    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[&]{reFreshPort();});
    timer->start(1000);

    ui->preview->hide();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_run_clicked()
{
    if(serialPort.portName().isEmpty())
    {
        qDebug()<<"没有可用端口";
        QMessageBox::critical(nullptr,"开启失败","没有可用端口",QMessageBox::Ok | QMessageBox::Cancel);
        return;
    }

    if(run == false)
    {
        if(!serialPort.open(QIODevice::ReadWrite))
        {
            qDebug()<<"开启失败"<<serialPort.portName();
            QMessageBox::critical(nullptr,"开启失败","请检测连接状态",QMessageBox::Ok | QMessageBox::Cancel);
            return;
        }
        openSerialPortQss();
    }
    else
    {
        serialPort.close();
        closeSerialPortQss();
    }
}


void Widget::on_refreshPort_clicked()
{   
    reFreshPort();
}


void Widget::on_port_currentTextChanged(const QString &arg1)
{
    serialPort.setPortName(arg1);
    port = arg1;
    qDebug()<<"当前端口号："<<serialPort.portName();
}


void Widget::on_pbs_currentTextChanged(const QString &arg1)
{
    serialPort.setBaudRate(arg1.toInt());
    qDebug()<<"当前波特率："<<serialPort.baudRate();
}


void Widget::on_data_currentTextChanged(const QString &arg1)
{
    serialPort.setDataBits(static_cast<QSerialPort::DataBits>(arg1.toInt()));
    qDebug()<<"当前数据位："<<serialPort.dataBits();
}

void Widget::on_stop_currentTextChanged(const QString &arg1)
{
    if(arg1 == "1.5")
        serialPort.setStopBits(QSerialPort::OneAndHalfStop);
    else
        serialPort.setStopBits(static_cast<QSerialPort::StopBits>(arg1.toInt()));
    qDebug()<<"当前停止位："<<serialPort.stopBits();
}

void Widget::on_parity_currentIndexChanged(int index)
{
    if(index == 0)
        serialPort.setParity(static_cast<QSerialPort::Parity>(0));
    else
        serialPort.setParity(static_cast<QSerialPort::Parity>(index+1));
    qDebug()<<"当前奇偶校验位："<<serialPort.parity();
}

bool Widget::eventFilter(QObject *obj, QEvent *event)
{

    if (obj == ui->textEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();

        QTextCursor cursor = ui->textEdit->textCursor();

        if(processKey(keyEvent))
            return true;

        if(key == Qt::Key_Up||key == Qt::Key_Down|| key == Qt::Key_Left||key == Qt::Key_Right)
            return false;

        if(cursor.position()<repos)
            return true;

        if(cursor.position()==repos &&key == Qt::Key_Backspace)
            return true;
        QTextCharFormat cf;
        cf.setForeground(QColor(tranText));
        cursor.setCharFormat(cf);
        ui->textEdit->setTextCursor(cursor);
        if(key == Qt::Key_Return)
        {
            qDebug()<<"enter";
            if(serialPort.isOpen())
            {
                QString allText = ui->textEdit->toPlainText();

                QString re = allText.mid(repos);

                cursor.movePosition(QTextCursor::End);
                cursor.insertText("\n");
                repos = cursor.position();

                serialPort.write(re.toUtf8()+"\n");


                qDebug()<<"发送了"<<re.toUtf8()+"\n";
                return true;
            }
            else
            {
                cursor.movePosition(QTextCursor::End);
                cursor.insertText("\n");
                repos = cursor.position();
                QMessageBox::critical(nullptr,"串口错误","请打开串口",QMessageBox::Ok);
                return true;
            }
        }

        return false;
    }

    return false;
}

void Widget::processStr(const QString &str)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    QTextCharFormat cf;
    cursor.movePosition(QTextCursor::End);
    cf.setForeground(QColor(reText));
    cursor.setCharFormat(cf);
    ui->textEdit->setTextCursor(cursor);
    foreach (auto ch, str) {
        switch(ch.unicode())
        {
        case '\a':
            QApplication::beep();
            break;
        case '\b':
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
            cursor.deletePreviousChar();
            break;
        case '\r':  //不支持
            break;
        case '\e':  //不支持
            break;
        case '\0':  //字符串结束
            break;
        case '\f':  //不支持
            break;
        case '\v':  //不支持
            break;
        case '\t':
            cursor.insertText("    ");
            break;
        default:
            cursor.insertText(ch);
            break;
        }
    }
    cursor.movePosition(QTextCursor::End);
    repos = cursor.position();

    cf.setForeground(QColor(tranText));
    cursor.setCharFormat(cf);
    ui->textEdit->setTextCursor(cursor);
}

bool Widget::processKey(QKeyEvent *event)
{
    if(event->modifiers()&Qt::ControlModifier)
    {
        switch (event->key()) {
        case Qt::Key_S:
            qDebug()<<"组合键：ctrl+s";
            return true;
        default:
            break;
        }
    }
    return false;
}

void Widget::openSerialPortQss()
{
    ui->run->setStyleSheet("QToolButton#run{background-color: rgb(250, 87, 87);border-radius: 5px;}");
    ui->run->setToolTip("stop");
    QIcon icon(":/image/end.png");
    ui->run->setIcon(icon);
    qDebug()<<"已开启"<<serialPort.portName();

    run = true;
}

void Widget::closeSerialPortQss()
{
    ui->run->setStyleSheet("QToolButton#run{background-color: rgb(0, 175, 0);border-radius: 5px;}");
    ui->run->setToolTip("start");
    QIcon icon(":/image/start.png");
    ui->run->setIcon(icon);
    qDebug()<<"已关闭";

    run = false;
}

void Widget::reFreshPort()
{
    ui->port->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        ui->port->addItem(info.portName());
    }
}


void Widget::on_cleanButton_clicked()
{
    ui->textEdit->clear();

    repos = 0;
}


void Widget::on_transTextButton_clicked()
{
    QFileInfo fileInfo(ui->lineEdit->text());
    QFile file(ui->lineEdit->text());
    bool i = false;

    if(!file.exists())
    {
        qDebug()<<"文件不存在";
        QMessageBox::critical(nullptr,"文件错误","文件不存在",QMessageBox::Ok);
        return;
    }

    QString suffix = fileInfo.suffix();

    foreach (auto&s, fileFormat) {
        if(suffix == s)
        {
            i = true;
            break;
        }
    }

    if(i == false)
    {
        QMessageBox::critical(nullptr,"文件错误","请重新选择",QMessageBox::Ok);
        return;
    }

    file.open(QIODevice::ReadOnly);
    QString txt(file.readAll());

    ui->textEdit->insertPlainText("\n");
    QTextCursor cursor = ui->textEdit->textCursor();
    QTextCharFormat cf;
    cursor.movePosition(QTextCursor::End);
    cf.setForeground(QColor(tranText));
    cursor.insertText("\n"+txt+"\n");
    repos = cursor.position();
    ui->textEdit->setTextCursor(cursor);

    if(serialPort.isOpen())
    {
        serialPort.write(txt.toUtf8()+"\n");
        qDebug()<<"发送成功\n"<<txt.toUtf8();
    }
    else
    {
        QMessageBox::critical(nullptr,"串口错误","请打开串口",QMessageBox::Ok);
    }

    file.close();
}


void Widget::on_fileButton_clicked()
{
    QString url = QFileDialog::getOpenFileName();

    ui->lineEdit->setText(url);
}


void Widget::on_setButton_clicked()
{
    set.show();
}

void Widget::handleError(QSerialPort::SerialPortError error)
{
    {
        QString errorMessage;
        switch (error) {
        case QSerialPort::DeviceNotFoundError:
            errorMessage = "设备未找到";
            break;
        case QSerialPort::PermissionError:
            errorMessage = "权限错误：请查看连接";
            closeSerialPortQss();
            serialPort.close();
            reFreshPort();
            port = ui->port->currentText();
            break;
        case QSerialPort::OpenError:
            errorMessage = "打开错误：请检查是否有其他程序占用";
            closeSerialPortQss();
            serialPort.close();
            break;
        case QSerialPort::WriteError:
            errorMessage = "写入错误";
            break;
        case QSerialPort::ReadError:
            errorMessage = "读取错误";
            break;
        case QSerialPort::ResourceError:
            errorMessage = "资源错误：请查看连接";
            closeSerialPortQss();
            serialPort.close();
            break;
        case QSerialPort::UnsupportedOperationError:
            errorMessage = "不支持的操作错误";
            break;
        case QSerialPort::TimeoutError:
            errorMessage = "超时错误";
            closeSerialPortQss();
            serialPort.close();
            break;
        default:
            return;
        }

        QMessageBox::critical(nullptr, "串口错误", errorMessage);
    }
}


void Widget::on_previewButton_clicked()
{
    static bool pre = false;

    if(pre == false)
    {
        ui->previewButton->setText("取消预览");
        ui->preview->show();
        QFileInfo fileInfo(ui->lineEdit->text());
        QFile file(ui->lineEdit->text());
        bool i = false;

        if(!file.exists())
        {
            qDebug()<<"文件不存在";
            QMessageBox::critical(nullptr,"文件错误","文件不存在",QMessageBox::Ok);
            return;
        }

        QString suffix = fileInfo.suffix();

        foreach (auto&s, fileFormat) {
            if(suffix == s)
            {
                i = true;
                break;
            }
        }

        if(i == false)
        {
            QMessageBox::critical(nullptr,"文件错误","请重新选择",QMessageBox::Ok);
            return;
        }

        file.open(QIODevice::ReadOnly);
        QString txt("预览中：\n"+file.readAll()+"\n");

        ui->preview->insertPlainText(txt);

        file.close();

    }
    else
    {
        ui->previewButton->setText("文件预览");
        ui->preview->clear();
        ui->preview->hide();
    }

    pre = !pre;

}

