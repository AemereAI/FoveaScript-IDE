#ifndef FOVEAWEBSITEINTERFACE_H
#define FOVEAWEBSITEINTERFACE_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QString>
#include <QDataStream>
#include <QPixmap>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QLabel>
#include <QThread>
//#include <QImageReader>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QHttpPart>
#include "foveapathsdefinitions.h"
#include <QStandardPaths>
#include <QNetworkDiskCache>
#include <QTime>
#include <QMutex>
enum class APPLIST_SEARCH_TYPE {CATEGORY= 0,DESCRIPTION=1,ALL=2};
enum class REQUEST_TYPE{APPLIST=0,APPCONTENT=1,APPAPI=2};

class FoveaWebsiteInterface : public QObject
{
    Q_OBJECT

public:
    FoveaWebsiteInterface();
    virtual ~FoveaWebsiteInterface();
    void forgotPasswordReset(QString user);
    void postDebugApp(QString user,QString session,QString file);
    void postFoveaApp(QString user,QString session,QString prog_name,QString ver,QString company,QString icon_url,QString category,QString descr,QString codefile);
    void requestAccountInfo(QString user,QString session);
    void requestDebugCurrentApp(QString user,QString session,QString request_param);
    void requestDebugOutput(QString user,QString session);
    void requestHTMLPage(QString link);
    QString next(enum REQUEST_TYPE type);
    const bool& available();
    void setCurrentRecieveMode(enum REQUEST_TYPE mode);
    void setReceiver(void (*sink)(QString));
    void setReceiver(void (*sink)(QByteArray));
    void setMediaReceiver(void (*sink)(QObject*,QString),QObject* icon);
    void setDestroyOnSuccess(bool destroy);
    void updateList();
    QString tempFolder();
    void setStorageFolder(QString folder);
    void wait();
    void forceWaitExit();
    void requestSignOut(QString user,QString session_);
    void stopLoad();
    void setOfflineMode(bool value);
    void setJSONmode(bool val);
public slots:
    void on_retrieved_image();
    void on_retrieved_list();
    void on_retrieved_content();
    void fileSize();
private:
    QNetworkAccessManager* net;
    QNetworkReply* reply;
    REQUEST_TYPE mode_selected;
    QList<QString>* apps_avail;
    QList<QString>* apps_contents;
    QList<QString>* apps_api;
    bool complete= false;
    QTemporaryDir *temp;
    QString imagesTemp;
    QString storageFolder;
    void (*rx)(QString);
    void (*rx2)(QByteArray);
    void (*rx_media)(QObject*,QString);
    QObject* rx_widget;
    static long photoID;
    QString type;
    bool destroys;
    bool compressed;
    QString public_image;
    qint64 content_size;
    QByteArray data;
    bool readAllContent;
    QNetworkDiskCache *disk_cache;
    bool stop;
    static bool offline_mode;
    bool json_mode;

};

#endif // FOVEAWEBSITEINTERFACE_H
