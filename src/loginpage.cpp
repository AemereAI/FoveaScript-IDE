#include "loginpage.h"
#include "ui_loginpage.h"


bool LoginPage::isMainVisible()
{
    return mainW->isVisible();
}
LoginPage::LoginPage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginPage)
{
    mainW = nullptr;
    ui->setupUi(this);
    ui->loginErrorLabel->setVisible(false);
    ui->forgot_label->setVisible(false);
    ui->fovea_icon->animate(Fovea3D::LOAD_BRING_IN);
    ui->user_name_edit->setFocus();
    ui->connectionErrorLabel->setVisible(false);
    alreadyActive = false;
    connect(timer(),SIGNAL(timeout()),this,SLOT(errorv2()));
    QMainWindow::setWindowIcon(QIcon(":/foveaicon_large.png"));
    net_forgot = nullptr;

    readSettings();

}

void LoginPage::writeSettings()
{
    QSettings settings("SuraTech Lab", "Fovea");

    settings.beginGroup("Settings");
    settings.setValue("user", user_last);
    settings.setValue("key", key);
    settings.endGroup();
}

void LoginPage::readSettings()
{
    QSettings settings("SuraTech Lab", "Fovea");

    settings.beginGroup("Settings");
    user_last = settings.value("user").toString();
    key = settings.value("key").toString();
    settings.endGroup();
    if(!user_last.trimmed().isEmpty() && !key.trimmed().isEmpty())
    {
        on_Sign_In_clicked();
        close();
    }
    else
        show();
}

LoginPage::~LoginPage()
{
    errorv2();
    delete ui;
}

void LoginPage::on_Sign_In_clicked()
{
    alreadyActive = true;
    if(ui->Sign_In->text() == tr("Loading Palettes..."))
        return;
    int expiry = 10;
    if(!(this->user_last.trimmed().isEmpty()) &&  !(this->key.trimmed().isEmpty()))
    {
        mainW = new fsIDE(nullptr,user_last,key);
        ui->fovea_icon->animate(Fovea3D::LOAD_BRING_IN);
        mainW->showMaximized();
        alreadyActive = false;
        net_conn = new QNetworkAccessManager(this);
        //QUrl palettes_url(tr(HOST_SITE) + tr(MY_PALETTES) + user_last +"/"+key+"/");

        return;
    }
    if((ui->user_name_edit->text() != "") || (ui->password_edit->text() != ""))
    {
        if(ui->stay_signed_in->isChecked())
        {
            expiry = 24*326;
        }
        QString user = ui->user_name_edit->text();
        QString pass = ui->password_edit->text();
        if(ui->user_name_edit->text() == "")
            user = "0";
        if(ui->password_edit->text() == "")
            pass="0";
        ui->fovea_icon->animate(Fovea3D::PROGRESS_ROTATE);
        QUrl login_url = QUrl(tr(HOST_SITE) +tr(LOGIN_SESSION) + QString::number(expiry) + "/");

        net_conn = new QNetworkAccessManager(this);
        QNetworkRequest request(login_url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QSslConfiguration conf = request.sslConfiguration();
        conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(conf);
        QJsonObject obj;
        obj["username"] = user;
        obj["password"] = pass;
        QJsonDocument doc(obj);
        QByteArray data = doc.toJson();
        connect(net_conn,SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )),this,SLOT(error(QNetworkReply*, const QList<QSslError> & )));
        rep = net_conn->post(request,data);
        connect(rep,SIGNAL(finished()),this,SLOT(on_retrieved()));
        timer()->start(1000*60);
        //Get user palettes
        //retrieve local palette files
        //Setup palettes widgets
    }
    else if ((ui->user_name_edit->text() == "") && (ui->password_edit->text() == "")){
    mainW = new fsIDE();
    ui->fovea_icon->animate(0);
    ui->fovea_icon->animate(Fovea3D::LOAD_GO);
    mainW->showMaximized();
    alreadyActive = false;
    close();
    }

}

void LoginPage::errorv2(QNetworkReply::NetworkError code)
{
    timer()->stop();
    ui->connectionErrorLabel->setVisible(true);
    ////qDebug() << rep->errorString();
    ////qDebug() << rep->readAll();
    ui->fovea_icon->animate(0);

}
void LoginPage::on_retrieved()
{

   timer()->stop();
   if(rep->error() == QNetworkReply::NoError){
                ui->connectionErrorLabel->setVisible(false);
                QString full_content = rep->readAll();
                bool succeeded = false;
                QJsonDocument doc =  QJsonDocument::fromJson(full_content.toUtf8());
                if(doc.isNull())
                    return;
                QJsonObject obj = doc.object();
                QString content = obj.value("output").toString();
                if(content.contains(QString("failed")))
                {
                    ui->loginErrorLabel->setText("Incorrect creditionals.  Please try again.");
                    ui->loginErrorLabel->setVisible(true);
                    ui->fovea_icon->animate(0);
                    return;
                }
                if(content.contains(QString("security")))
                {
                    ui->loginErrorLabel->setText("(Security) Account temporarity locked.");
                    ui->loginErrorLabel->setVisible(true);
                    ui->fovea_icon->animate(0);
                    return;
                }
                else
                {
                    QRegularExpression regex("[0-9a-z]+");
                    key = regex.match(content).captured(0);
                    if(key.length() >0)
                    {
                        succeeded = true;
                    }
                }
                if(succeeded)
                {
                   user_last = ui->user_name_edit->text();
                   writeSettings();
                   QString user = ui->user_name_edit->text();
                   if(mainW == nullptr)
                       mainW = new fsIDE(nullptr,user,key);
                   ui->fovea_icon->animate(Fovea3D::LOAD_GO);
                   ui->Sign_In->setText(tr("Sign In"));
                   mainW->showMaximized();
                   alreadyActive = false;
                   close();
                }
   }
   else {
       ui->connectionErrorLabel->setVisible(true);
       ui->fovea_icon->animate(0);

   }
}

void LoginPage::onfinal_retrieved()
{


}

void LoginPage::on_password_edit_returnPressed()
{
    on_Sign_In_clicked();
}

void LoginPage::on_user_name_edit_returnPressed()
{
    on_Sign_In_clicked();

}
void LoginPage::error(QNetworkReply* rep,const QList<QSslError> &errors)
{
    QStringList strings;
    for (int i = 0;i< errors.length();i++) {
        strings.append(errors.at(i).errorString());
        ////qDebug() << strings.at(i);
    }
}

QTimer* LoginPage::timer()
{
    static QTimer my_timer;
    return &my_timer;
}

void LoginPage::on_register_now_clicked()
{
    QDesktopServices::openUrl(QUrl("https://dev.suratechlab.com/registernow/"));
}

void LoginPage::on_forgot_pass_clicked()
{
    if(!(ui->user_name_edit->text().isEmpty()))
    {
        ui->forgot_label->setVisible(false);
        if(net_forgot == nullptr)
            net_forgot = new FoveaWebsiteInterface;
        net_forgot->forgotPasswordReset(ui->user_name_edit->text().trimmed());
        QMessageBox msgBox;
        msgBox.setText(ui->user_name_edit->text()+ ", an email has been sent to you.");
        msgBox.exec();
    }
    else
    {
       ui->forgot_label->setVisible(true);
    }

}
