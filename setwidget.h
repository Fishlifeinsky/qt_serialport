#ifndef SETWIDGET_H
#define SETWIDGET_H

#include <QWidget>
#include <QString>

namespace Ui {
class setWidget;
}

class setWidget : public QWidget
{
    Q_OBJECT

public:
    explicit setWidget(QWidget *parent = nullptr);
    ~setWidget();

public:
    Ui::setWidget *ui;
private slots:

    void on_tranCloorButton_clicked();

    void on_reCloorButton_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_textEdit_textChanged();

private:
    QJsonDocument *jsonDoc;
    QStringList *fileFormat;
    QString tranText;
    QString reText;

private:
    void readJson();
    void reColorText();

public:
    QString tranTextColor() const {return tranText;}
    QString reTextColor() const {return reText;}
    QStringList fileFormatList() const {return *fileFormat;}

signals:
    void setChange();
};

#endif // SETWIDGET_H
