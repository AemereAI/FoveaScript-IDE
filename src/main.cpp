#include "loginpage.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << QStyleFactory::keys();
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QFontDatabase::addApplicationFont(":/AbyssinicaSIL-Regular.ttf");
    QFont font("Power Geez Unicode1",10);
    font.setStyleHint(QFont::StyleHint::TypeWriter);
    QApplication::setFont(font);
    a.setApplicationName("SuraTech Lab Fovea");
    LoginPage landing;
    return a.exec();
}
