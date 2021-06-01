#pragma once

#include <QString>

QString getTitleBarQSS() {

    QString titleQSS = "QWidget#titleBar{\
	    min-height:24px;\
	    max-height: 24px;\
        background:rgb(45,45,48);\
        border: none;\
    }\
    QWidget#centralwidget{\
        background:rgb(00,66,77);\
    }\
    QPushButton#minimizeButton{\
        min-width: 24px;\
        max-width: 24px;\
        min-height: 24px;\
        max-height: 24px;\
        border-image: url(:/img/min.png);\
    }\
    QPushButton#maximizeButton{\
        min-width: 24px;\
        max-width: 24px;\
        min-height: 24px;\
        max-height: 24px;\
        border-image: url(:/img/max.png);\
    }\
    QPushButton#maximizeButton:checked{\
        min-width: 24px;\
        max-width: 24px;\
        min-height: 24px;\
        max-height: 24px;\
        border-image: url(:/img/restore.png);\
    }\
    QPushButton#closeButton{\
        min-width: 24px;\
        max-width: 24px;\
        min-height: 24px;\
        max-height: 24px;\
        border-image: url(:/img/closeBtn.png);\
    }\
    QPushButton#closeButton:hover{\
        background-color: rgb(232,17,35);\
    }";
    return titleQSS;
}