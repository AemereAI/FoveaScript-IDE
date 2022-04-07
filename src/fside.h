#ifndef FSIDE_H
#define FSIDE_H

#include <QMainWindow>
#include "foveawebsiteinterface.h"
#include <QTemporaryFile>
#include <QTimer>
#include <QKeyEvent>
#include "syntaxhighlighter.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <QTemporaryDir>
#include <QMessageBox>
#include <QtWebEngineWidgets/QWebEngineView>

QT_BEGIN_NAMESPACE
namespace Ui { class fsIDE; }
QT_END_NAMESPACE

class fsIDE : public QMainWindow
{
    Q_OBJECT

public:
    fsIDE(QWidget *parent = nullptr,QString user = "",QString key = "");
    static void inputUploadResponse(QString html);
    static void inputDebugOutput(QString html);
    void setUser(QString user,QString sessionid);
    ~fsIDE();
public slots:
    void updateContents();
    void updateOutput();

private slots:
    void on_run_button_clicked();
    void refresh();
    void on_upload_button_clicked();
    void on_func_code_edit_textChanged();
    void onlog_out();
    void on_signout_button_clicked();
    void on_complete();
    void on_checkBox_2_stateChanged(int arg1);

private:
    void keyReleaseEvent(QKeyEvent *event);
    QString checkForErrors(QString code);
    static QString username;
    static QString sessionkey;
    static QString request_details;
    static QString response_request;
    static QTimer* timer();
    static QTimer* refreshTimer();


private:
    Ui::fsIDE *ui;
    FoveaWebsiteInterface *interface_;
    QString my_template;
    SyntaxHighlighter *highlighter;
    QNetworkAccessManager *net_conn;
    QNetworkReply *rep;
    QTemporaryDir *dir;
    int upload_count;
    static fsIDE *ide_static;
    int last_pos;
    QString before;
    bool disable_AI;
    QWebEngineView *view;
};
#endif // FSIDE_H
