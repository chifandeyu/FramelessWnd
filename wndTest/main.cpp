
#include <QtWidgets/QApplication>

#ifdef _WIN32
#include "FramelessWnd.h"
#else
#include "widget.h"
#endif

#ifdef __APPLE__
#include "OSXHideTitleBar.h"
#endif

#include "titleBarStyle.h"

int main(int argc, char *argv[])
{
    //This has the app draw at HiDPI scaling on HiDPI displays, usually two pixels for every one logical pixel
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    //This has QPixmap images use the @2x images when available
    //See this bug for more details on how to get this right: https://bugreports.qt.io/browse/QTBUG-44486#comment-327410
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);
    //Create the true app widget 
    //add widget
    QWidget *appWidget = new QWidget();
    appWidget->setObjectName("appWidget");
    appWidget->setStyleSheet("QWidget#appWidget{background:rgb(44,66,88);}");
    FramelessWnd wnd(appWidget);
    wnd.setTitleStyle(getTitleBarQSS());
    //A common feature is to save your app's geometry on close such that you can draw in the same place on relaunch
    //Thus this project supports specifying the X/Y/Width/Height in a cross-platform manner
    //int windowXPos, windowYPos, windowWidth, windowHeight;
    //windowXPos = 10;
    //windowYPos = 10;
    //windowWidth = 1024;
    //windowHeight = 768;
    //wnd.setGeometry(windowXPos, windowYPos, windowWidth, windowHeight);
    wnd.show();
    return a.exec();
}
