#include "setwidget.h"
#include "ui_setwidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QColorDialog>

#define SETJSON "{  \"tranText\":\"#00FF00\",\
                    \"reText\":\"#FF0000\",\"file format\":[\"txt\",\"json\"]}"

setWidget::setWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::setWidget)
{
    ui->setupUi(this);
    this->show();

    QFile file(QCoreApplication::applicationDirPath()+"/set.json");
    qDebug()<<file.fileName();

    fileFormat = new QStringList();

    if(!file.exists())
    {
        file.open(QIODevice::WriteOnly);       

        jsonDoc = new QJsonDocument(QJsonDocument::fromJson(SETJSON));

        if(jsonDoc->isNull())
            qDebug()<<"fild json";
        else
        {
            qDebug()<<jsonDoc->toJson();
            file.write(jsonDoc->toJson());
            file.close();
        }

        *fileFormat<<"txt"<<"json";
        tranText = "#00FF00";
        reText = "#FF0000";
        ui->textEdit->setPlainText("txt&json");
        ui->tranCloorButton->setStyleSheet("background-color: " + tranText + ";");
        ui->reCloorButton->setStyleSheet("background-color: " + reText + ";");
        reColorText();
    }
    else
    {
        file.open(QIODevice::ReadWrite);
        QString json = file.readAll();
        file.close();

        jsonDoc = new QJsonDocument(QJsonDocument::fromJson(json.toUtf8()));

        if(jsonDoc->isNull())
            qDebug()<<"fild json";
        else
        {
            qDebug()<<jsonDoc->toJson();
            file.write(jsonDoc->toJson());
            file.close();
        }

        readJson();
        reColorText();
    }
}

setWidget::~setWidget()
{
    delete ui;
}

void setWidget::on_tranCloorButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white,nullptr,"tranColor");

    if(color.name() == tranText)
        return;
    else
        tranText = color.name();

    ui->tranCloorButton->setStyleSheet("background-color: " + tranText + ";");

}


void setWidget::on_reCloorButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white,nullptr,"reColor");

    if(color.name() == reText)
        return;
    else
        reText = color.name();   

    ui->reCloorButton->setStyleSheet("background-color: " + reText + ";");
}


void setWidget::on_pushButton_clicked()
{
    QJsonObject rootObj = jsonDoc->object();

    QJsonArray ff;

    QStringList sl = ui->textEdit->toPlainText().split("&");

    foreach (auto& str, sl) {
        ff<<str;
    }

    rootObj["file format"] = ff;
    rootObj["reText"] = reText;
    rootObj["tranText"] = tranText;
    delete jsonDoc;
    jsonDoc = new QJsonDocument(rootObj);


    QFile file(QCoreApplication::applicationDirPath()+"/set.json");

    file.open(QIODevice::ReadWrite|QIODevice::Truncate);
    file.write(jsonDoc->toJson());
    file.close();

    emit setChange();

    this->hide();
}


void setWidget::on_pushButton_2_clicked()
{
    QFile file(QCoreApplication::applicationDirPath()+"/set.json");
    file.open(QIODevice::ReadWrite);

    delete jsonDoc;
    jsonDoc = new QJsonDocument(QJsonDocument::fromJson(file.readAll()));
    file.close();

    readJson();
    reColorText();

    this->hide();
}

void setWidget::readJson()
{
    QJsonObject rootObj = jsonDoc->object();

    tranText = rootObj.value("tranText").toString();
    reText = rootObj.value("reText").toString();

    QJsonArray ff = rootObj.value("file format").toArray();

    fileFormat->clear();
    ui->textEdit->clear();

    QString t;
    for(int i = 0;i<ff.count();i++)
    {
        *fileFormat<<ff.at(i).toString();
        t+=ff.at(i).toString();
        if(i<ff.count()-1)
            t+="&";
    }

    ui->textEdit->setPlainText(t);
    ui->tranCloorButton->setStyleSheet("background-color: " + tranText + ";");
    ui->reCloorButton->setStyleSheet("background-color: " + reText + ";");
}

void setWidget::reColorText()
{
    QString text = ui->textEdit->toPlainText();

    QTextCursor cursor(ui->textEdit->textCursor());

    QTextCharFormat Format;
    Format.setForeground(Qt::black);

    cursor.setPosition(0);
    cursor.setPosition(text.length(), QTextCursor::KeepAnchor);
    cursor.setCharFormat(Format);

    int index = text.indexOf('&');
    Format.setForeground(Qt::red);
    while (index != -1) {
        cursor.setPosition(index);
        cursor.setPosition(index + 1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(Format);
        index = text.indexOf('&', index + 1);
    }
    cursor.setPosition(text.length());
}


void setWidget::on_textEdit_textChanged()
{
    ui->textEdit->blockSignals(true);

    reColorText();

    ui->textEdit->blockSignals(false);
}

