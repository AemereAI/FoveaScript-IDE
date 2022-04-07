#ifndef LOGINPAGE_H
#define LOGINPAGE_H
#include "foveapathsdefinitions.h"
#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QTemporaryFile>
#include <QDataStream>
#include <QTemporaryDir>
#include <QTextCodec>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include "fside.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDesktopServices>
#include "foveawebsiteinterface.h"
#include <QMessageBox>
namespace Ui {
class LoginPage;
}
//int on_extract_entry(const char *filename, void *arg);

class LoginPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    bool isMainVisible();
    void readSettings();
    void writeSettings();
    ~LoginPage();

public slots:
    void on_Sign_In_clicked();

    void on_retrieved();

    void onfinal_retrieved();

    void on_password_edit_returnPressed();

    void error(QNetworkReply* rep,const QList<QSslError> &errors);
    void errorv2(QNetworkReply::NetworkError e = QNetworkReply::NetworkError::TimeoutError);

private slots:
    void on_user_name_edit_returnPressed();
    void on_register_now_clicked();

    void on_forgot_pass_clicked();

private:
    Ui::LoginPage *ui;
    static QTimer* timer();
    QNetworkAccessManager* net_conn;
    QNetworkReply *rep;
    QNetworkReply *rep2;
    fsIDE* mainW;
    QString key;
    bool alreadyActive;
    QString user_last;
    FoveaWebsiteInterface *net_forgot;
};

#endif // LOGINPAGE_H
