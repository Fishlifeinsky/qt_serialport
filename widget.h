#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QKeyEvent>
#include <QStringList>
#include "setwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_run_clicked();

    void on_refreshPort_clicked();

    void on_port_currentTextChanged(const QString &arg1);

    void on_pbs_currentTextChanged(const QString &arg1);

    void on_data_currentTextChanged(const QString &arg1);

    void on_stop_currentTextChanged(const QString &arg1);

    void on_parity_currentIndexChanged(int index);

    void on_cleanButton_clicked();

    void on_transTextButton_clicked();

    void on_fileButton_clicked();

    void on_setButton_clicked();

    void on_previewButton_clicked();

public slots:
    void handleError(QSerialPort::SerialPortError error);

private:
    Ui::Widget *ui;
    QSerialPort serialPort;
    QString port;
    setWidget set;
    QStringList fileFormat;
    QString tranText;
    QString reText;
    qint64 repos;
    bool run = false;
    QTimer* timer;
public:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void processStr(const QString &str);
    bool processKey(QKeyEvent *event);
    void openSerialPortQss();
    void closeSerialPortQss();
    void reFreshPort();
};
#endif // WIDGET_H
