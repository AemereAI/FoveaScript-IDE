#include "foveawebsiteinterface.h"
#include "loginpage.h"

long FoveaWebsiteInterface::photoID = 0;
bool FoveaWebsiteInterface::offline_mode = false;
FoveaWebsiteInterface::FoveaWebsiteInterface()
{
    net = new QNetworkAccessManager();
    apps_avail = new QList<QString>();
    apps_contents = new QList<QString>();
    apps_api = new QList<QString>();
    temp = new QTemporaryDir();
    //imagesTemp = temp->path();
    imagesTemp = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    destroys = false;
    compressed = false;
    disk_cache = new QNetworkDiskCache(this);
    disk_cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    disk_cache->setMaximumCacheSize(1024*1024*1024);
    net->setCache(disk_cache);
    stop = false;
    complete = true;
    json_mode = false;
    rx2 = nullptr;
}
FoveaWebsiteInterface::~FoveaWebsiteInterface()
{
    complete = true;
}

void FoveaWebsiteInterface::requestHTMLPage(QString link)
{
    QUrl url(link);
    //qDebug() << url;
    content_size = 0;
    data.clear();
    readAllContent = false;
    QNetworkRequest r(url);
    QSslConfiguration conf = r.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    r.setSslConfiguration(conf);
    r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    r.setRawHeader("User-Agent","Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:83.0) Gecko/20100101 Firefox/83.0");
    QObject::connect(net, SIGNAL(finished(QNetworkReply*)),this, SLOT(on_retrieved_content()));
    reply = net->get(r);


}

void FoveaWebsiteInterface::forgotPasswordReset(QString user)
{
    QUrl url(QString(HOST_SITE) + QString(FORGOT_PASS) + user + "/");
    QNetworkRequest r(url);
    QSslConfiguration conf = r.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    r.setSslConfiguration(conf);
   // QObject::connect(net, SIGNAL(finished(QNetworkReply*)),this, SLOT(on_retrieved_content()));
    reply = net->get(r);
}



void FoveaWebsiteInterface::requestSignOut(QString user,QString sessionid)
{
    QUrl url(QString(HOST_SITE)+QString(SIGNOUT)+user+"/"+sessionid+"/");
    QNetworkRequest r(url);
    QSslConfiguration conf = r.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    r.setSslConfiguration(conf);
    r.setRawHeader("User-Agent","Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:83.0) Gecko/20100101 Firefox/83.0");
    QObject::connect(net, SIGNAL(finished(QNetworkReply*)),this, SLOT(on_retrieved_content()));
    reply = net->get(r);

}



void FoveaWebsiteInterface::postDebugApp(QString user, QString session, QString file)
{
    QUrl post(QString(HOST_SITE)+QString(DEBUG_UPLOAD)+user+"/"+session+"/");
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QFile* f = new QFile(file);
    try{
        f->open(QIODevice::ReadOnly);
    }
    catch(...){
        ////qDebug() << "File input Error";
        return;
    }
    QHttpPart *filePart = new QHttpPart();
    filePart->setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"debug_script_file\"; filename=\""+f->fileName()+"\""));
    filePart->setHeader(QNetworkRequest::ContentLengthHeader,f->size());

    filePart->setBodyDevice(f);
    multiPart->append(*filePart);
    f->setParent(multiPart);

    QNetworkRequest req(post);
    QSslConfiguration conf = req.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(conf);
    reply = net->post(req,multiPart);
    QObject::connect(reply,SIGNAL(finished()),this,SLOT(on_retrieved_content()));
    multiPart->setParent(reply);

}

void FoveaWebsiteInterface::requestAccountInfo(QString user, QString session)
{
    if(user.isEmpty())
        return;
    QUrl api = QUrl(QString(HOST_SITE) + QString(ACCOUNT) + user + QString("/")+session+QString("/"));
    QNetworkRequest request(api);
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);
    reply = net->get(request);
    QObject::connect(reply,SIGNAL(finished()),this,SLOT(on_retrieved_content()));
}

void FoveaWebsiteInterface::requestDebugCurrentApp(QString user, QString session,QString user_request)
{
    if(user_request == "")
        user_request = "_";
    QUrl api = QUrl(QString(HOST_SITE) + QString(DEBUG_RUN) + user + tr("/") + session + tr("/") +user_request.replace(" ","+") + tr("/"));
    //qDebug() << api;
    QNetworkRequest request(api);
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);
    reply = net->get(request);
    QObject::connect(reply,SIGNAL(finished()),this,SLOT(on_retrieved_content()));
}

void FoveaWebsiteInterface::requestDebugOutput(QString user, QString session)
{

    QUrl api = QUrl(QString(HOST_SITE) + QString(DEBUG_OUTPUT) + user + tr("/") + session + tr("/"));
    //qDebug() << api;
    QNetworkRequest request(api);
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);
    reply = net->get(request);
    QObject::connect(reply,SIGNAL(finished()),this,SLOT(on_retrieved_content()));
}

void FoveaWebsiteInterface::on_retrieved_list()
{
    QString cont = reply->readAll();
    switch(mode_selected)
    {
    case REQUEST_TYPE::APPAPI:
        apps_api->append(cont);
        break;
    case REQUEST_TYPE::APPLIST:
        apps_avail->append(cont);
        break;
    default:
        break;
    }
    updateList();
}

QString FoveaWebsiteInterface::tempFolder()
{
    return imagesTemp;
}

void FoveaWebsiteInterface::on_retrieved_image()
{
if(QNetworkReply::NoError == reply->error())
{
    QByteArray data = reply->readAll();
    QString path_i = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if(compressed)
    {
    //QTemporaryDir *d = new QTemporaryDir;
   // QString loc = d->path() + "/temp.zip";
     QString base_dir =  path_i + "/" + QString::number(photoID-1) + "/";
    QString loc = base_dir +"temp.zip";
    QDir dir_del(base_dir);
    dir_del.removeRecursively();
    QDir().mkdir(base_dir);

    QFile t(loc);
    t.open(QIODevice::WriteOnly);
    QDataStream stream(&t);

    if(data.length() > 0)
    {
        stream.writeRawData(data.data(),data.length());
    }
    t.flush();
    t.close();

    int arg = 2;
    //zip_extract(loc.toUtf8().data(), base_dir.toUtf8().data(), on_extract_entry, &arg);
    //QString path=  d->path() + "/" + public_image;
    QString filters = "*.fp";
    QDir dir(base_dir);
    QDirIterator iter(base_dir, QStringList() << filters, QDir::Files, QDirIterator::NoIteratorFlags);

    while (iter.hasNext())
    {
        iter.next();
        rx_media(rx_widget,iter.filePath());

    }

    ////qDebug() << path;
    complete =true;
    if(destroys)
    {
        this->deleteLater();
    }
    return;
    }

    QPixmap map;
    map.loadFromData(data,type.toLocal8Bit().data());
    QFile imageFile(imagesTemp + "/" + QString::number(photoID - 1) + "." + type);
    imageFile.open(QIODevice::WriteOnly);
    /* int size= */imageFile.write(data.data(),data.length());
    imageFile.close();
    ////qDebug() << ( imagesTemp + "/" + QString::number(photoID - 1)+ "." + type);
    rx_media(rx_widget,imagesTemp + "/" + QString::number(photoID - 1)+ "." + type);
    complete =true;
    if(destroys)
    {
        this->deleteLater();
    }
}
else
{
    //qDebug() << reply->errorString();
    complete = true;
    rx_media(rx_widget,"");
return;
}
}

void FoveaWebsiteInterface::setMediaReceiver(void (*sink)(QObject*,QString),QObject* icon)
{
    rx_widget= icon;
    rx_media = sink;
}

void FoveaWebsiteInterface::updateList()
{
    switch(mode_selected)
    {

    case REQUEST_TYPE::APPLIST:
            (*rx)(next(mode_selected));
        break;
    case REQUEST_TYPE::APPAPI:
            (*rx)(next(mode_selected));
        break;
    default:
        break;
    }
}
const bool& FoveaWebsiteInterface::available()
{
    return complete;
}

void FoveaWebsiteInterface::on_retrieved_content()
{
    if(stop)
    {
        complete = true;
        stop = false;
        return;
    }

    if(QNetworkReply::NoError == reply->error())
    {
        //qDebug() << reply->bytesAvailable();
        content_size = reply->bytesAvailable();
        data = reply->readAll();

        if(json_mode)
        {
            if(rx2 != nullptr)
               (*rx2)(data);
            complete = true;
            json_mode = false;
        }
        else
        {
            if(rx != nullptr)
               (*rx)(QString(data));
            complete = true;
        }



    }
    else
    {
        QString failed_data = "Fovea Connection Failed: " + reply->errorString();
        if(json_mode)
        {
            if(rx2 != nullptr)
               (*rx2)(data);
            complete = true;
            json_mode = false;
        }
        else
        {
        if(rx != nullptr)
            (*rx)(failed_data);
        complete = true;
    }
    }
    if(destroys)
    {
            this->deleteLater();
    }

}
void FoveaWebsiteInterface::setDestroyOnSuccess(bool destroy)
{
    destroys = destroy;
}
QString FoveaWebsiteInterface::next(REQUEST_TYPE type)
{
    QString response;
    switch(type)
    {
        case REQUEST_TYPE::APPLIST:
         response = apps_avail->at(0);
         apps_avail->removeAt(0);
            break;
        case REQUEST_TYPE::APPAPI:
         response = apps_api->at(0);
         apps_api->removeAt(0);
        break;
        default:
          response =  QString();
        break;
    }
    return response;

}

void FoveaWebsiteInterface::setCurrentRecieveMode(REQUEST_TYPE mode)
{
    mode_selected = mode;
}
void FoveaWebsiteInterface::setReceiver(void (*sink)(QString))
{
    rx = sink;

}

void FoveaWebsiteInterface::setReceiver(void (*sink)(QByteArray))
{
    rx2 = sink;

}

void FoveaWebsiteInterface::forceWaitExit()
{
    complete = true;
}

void FoveaWebsiteInterface::wait()
{
    QElapsedTimer t;
    t.start();
 while(complete == false)
 {
   // QCoreApplication::processEvents();
    if(t.elapsed() > 60000)
        break;
 }
}

void FoveaWebsiteInterface::fileSize()
{
    //qDebug() << "Content Length: " << reply->header(QNetworkRequest::ContentLengthHeader).toString();        QString data = reply->readAll();

}

void FoveaWebsiteInterface::stopLoad()
{
    stop = true;
}


void FoveaWebsiteInterface::postFoveaApp(QString user,QString session,QString prog_name, QString ver, QString company, QString icon_url, QString category, QString descr, QString codefile)
{
    complete = false;
    QUrl post(QString(HOST_SITE)+QString(UPLOAD_APP)+user+"/"+session+"/");
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart *progNamePart = new QHttpPart();
    progNamePart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"program_name\""));
    progNamePart->setBody(prog_name.toUtf8().replace(" ", "+"));
    multiPart->append(*progNamePart);
    QHttpPart *versionPart = new QHttpPart();
    versionPart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"version_info\""));
    versionPart->setBody(ver.toUtf8());
    multiPart->append(*versionPart);
    QHttpPart *companyPart = new QHttpPart();
    companyPart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"company_name\""));
    companyPart->setBody(company.toUtf8());
    multiPart->append(*companyPart);
    QHttpPart *iconPart = new QHttpPart();
    iconPart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"icon_url\""));
    iconPart->setBody(icon_url.toUtf8());
    multiPart->append(*iconPart);
    QHttpPart *catPart = new QHttpPart();
    catPart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"app_category\""));
    catPart->setBody(category.toUtf8());
    multiPart->append(*catPart);
    QHttpPart *descPart = new QHttpPart();
    descPart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"app_description\""));
    descPart->setBody(descr.toUtf8());
    multiPart->append(*descPart);
    QFile* f = new QFile(codefile);
    try{
        f->open(QIODevice::ReadOnly);
    }
    catch(...){
        //qDebug() << "File input Error";
        return;
    }
    QHttpPart *filePart = new QHttpPart();
    filePart->setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"script_file\"; filename=\""+f->fileName()+"\""));
    filePart->setHeader(QNetworkRequest::ContentLengthHeader,f->size());

    filePart->setBodyDevice(f);
    multiPart->append(*filePart);
    f->setParent(multiPart);

    QNetworkRequest req(post);
    QSslConfiguration conf = req.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(conf);
   // req.setHeader(QNetworkRequest::ContentTypeHeader,"multipart/form-data;");
    reply = net->post(req,multiPart);
    QObject::connect(reply,SIGNAL(finished()),this,SLOT(on_retrieved_content()));
    multiPart->setParent(reply);
}
