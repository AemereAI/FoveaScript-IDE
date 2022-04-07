QT       += core gui
QT       += network webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

win32: LIBS    += -lopengl32
win32: LIBS    += -L$$PWD/libcrypto
win32: LIBS    += -L$$PWD/libssl

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = fovea.ico

SOURCES += \
    codeeditor.cpp \
    fovea3d.cpp \
    foveawebsiteinterface.cpp \
    loginpage.cpp \
    main.cpp \
    fside.cpp \
    objreader.cpp \
    syntaxhighlighter.cpp 

HEADERS += \
    codeeditor.h \
    fovea3d.h \
    fovealexical.h \
    foveapathsdefinitions.h \
    foveawebsiteinterface.h \
    fside.h \
    loginpage.h \
    objreader.h \
    syntaxhighlighter.h 

FORMS += \
    fside.ui \
    loginpage.ui 


DISTFILES += \
    FoveaScriptIDE.pro.user

RESOURCES += \
    rsc.qrc
