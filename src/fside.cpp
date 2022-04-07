#include "fside.h"
#include "ui_fside.h"
#include "loginpage.h"
QString fsIDE::username;
QString fsIDE::sessionkey;
QString fsIDE::request_details;
QString fsIDE::response_request;
fsIDE* fsIDE::ide_static;

fsIDE::fsIDE(QWidget *parent,QString user,QString key)
    : QMainWindow(parent)
    , ui(new Ui::fsIDE)
{
    ui->setupUi(this);
    statusBar()->hide();
    connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
    connect(refreshTimer(),SIGNAL(timeout()),this,SLOT(updateOutput()));
    setUser(user,key);
    interface_ = nullptr;
    highlighter = new SyntaxHighlighter(ui->func_code_edit->document());
    net_conn = new QNetworkAccessManager();
    connect(net_conn, &QNetworkAccessManager::finished,
            this, &fsIDE::updateContents);
    QMainWindow::setWindowIcon(QIcon(":/foveaicon_large.png"));
    dir = new QTemporaryDir;
    ide_static = this;
    last_pos = 0;
    disable_AI = false;
    view = new QWebEngineView;
    view->setWindowTitle("Fovea Web View");
}

void fsIDE::onlog_out()
{
    QSettings settings("SuraTech Lab", "Fovea");

    settings.beginGroup("Settings");
    settings.setValue("user", "");
    settings.setValue("key", "");
    settings.endGroup();
}
fsIDE::~fsIDE()
{
    delete ui;
}


void fsIDE::refresh()
{
    timer()->stop();
    if(ui->output_response_edit->toPlainText().startsWith("Line "))
        return;
    ui->output_response_edit->setText(response_request);
    QRegularExpression r(QString("<request>.*?</request>"));
    QRegularExpressionMatch m = r.match(response_request);
    QString rep = response_request.replace(m.captured(0),"");
    view->setHtml(rep);
    view->show();

}

void fsIDE::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Back)
    {
            event->setAccepted(true);
            hide();
    }
}

QTimer* fsIDE::timer()
{
    static QTimer my_timer;
    return &my_timer;
}

QTimer* fsIDE::refreshTimer()
{
    static QTimer my_timer;
    return &my_timer;
}

void fsIDE::setUser(QString user, QString sessionid)
{
    username = user;
    sessionkey = sessionid;
}

void fsIDE::on_run_button_clicked()
{
    if(username.isEmpty() || sessionkey.isEmpty())
    {
        return;
    }
    QString error = checkForErrors(ui->func_code_edit->toPlainText());
    if(error.length() > 0)
    {
        ui->output_response_edit->setText(error);
        return;
    }
    ui->output_response_edit->setText("");
    QTemporaryFile script;
    script.open();
    QString filepath = script.fileName();
    QString script_func = "function test_func(Keyword){" + ui->func_code_edit->toPlainText() + tr("}");
    script.write(script_func.toUtf8().data());
    script.close();
    interface_ = new FoveaWebsiteInterface;
    interface_->setCurrentRecieveMode(REQUEST_TYPE::APPCONTENT);
    interface_->setReceiver(&fsIDE::inputUploadResponse);
    interface_->setDestroyOnSuccess(true);
    if(!(ui->keyword_value_edit->text().trimmed().isEmpty()))
    {
    request_details = ui->keyword_value_edit->text().split(",").join("_00_");
    }
    else {
        request_details = "";
    }
    refreshTimer()->start(5000);
    interface_->postDebugApp(username,sessionkey,filepath);

}

void fsIDE::inputUploadResponse(QString html)
{
    if(html.contains("Successfully prepared for debugging"))
    {
        FoveaWebsiteInterface *interface_ = new FoveaWebsiteInterface;
        interface_->setCurrentRecieveMode(REQUEST_TYPE::APPCONTENT);
        interface_->setReceiver(&fsIDE::inputDebugOutput);
        interface_->setDestroyOnSuccess(true);
        interface_->requestDebugCurrentApp(username,sessionkey,request_details);
    }
    else
    {
        QMessageBox msg;
        msg.setText(html);
        msg.exec();
    }
}

void fsIDE::inputDebugOutput(QString html)
{
    if(html.startsWith("Fovea Connection Failed: Error transferring https:/dev.suratechlab.com/debugcurrentscript/"))
        response_request = "Error in script, check the code!";
    else if (html != "" && html != "Invalid Request")
        response_request = html;
    else
        return;
    timer()->start(1);
}


void fsIDE::on_complete()
{
    QMessageBox msg;
    msg.setText(QString(rep->readAll()));
    msg.exec();
}

void fsIDE::on_upload_button_clicked()
{
    if(!username.isEmpty() && !(ui->plugin_name_edit->text().trimmed().isEmpty()))
    {
    QString error = checkForErrors(ui->func_code_edit->toPlainText());
    if(error.length() > 0)
    {
        ui->output_response_edit->setText(error);
        return;
    }
    else
        ui->output_response_edit->setText("");
    interface_ = new FoveaWebsiteInterface;
    interface_->setCurrentRecieveMode(REQUEST_TYPE::APPCONTENT);
    interface_->setReceiver(&fsIDE::inputUploadResponse);
    QString icon_url = "https://dev.suratechlab.com";

    QFile temp(":/template.fs");
    if(!temp.open(QIODevice::ReadOnly))
    {
        return;
    }
    QString template_file = QString::fromLatin1(temp.readAll()) + ui->func_code_edit->toPlainText() + "}";
    QString file_name(dir->path() + "/" + QString::number(upload_count++));
    QFile f(file_name);
    if(!f.open(QIODevice::WriteOnly))
        return;
    f.write(template_file.toLatin1());
    f.close();
    interface_->postFoveaApp(username,sessionkey,ui->plugin_name_edit->text(),"1.0",username,icon_url,"All","SuraTech Lab Plugin",file_name);
    }
    else
    {
        if(username.isEmpty())
            ui->output_response_edit->setText("Sorry, login to upload");
        else
            ui->output_response_edit->setText("Please enter a plugin name");
    }
}


void fsIDE::on_func_code_edit_textChanged()
{
    last_pos = ui->func_code_edit->textCursor().position();
    before = ui->func_code_edit->toPlainText().left(last_pos);
    QStringList before_line = before.split(";");

    if(before.endsWith(";") && !disable_AI)
    {
    QUrl ai_engine = QUrl(tr(HOST_SITE) +tr(AI_ENGINE));
    QNetworkRequest request(ai_engine);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);
    QJsonObject obj;
    obj["code"] = before_line.at(before_line.count() -2);
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();
    rep = net_conn->post(request,data);
    }
}
void fsIDE::updateContents()
{
    if(QNetworkReply::NoError == rep->error())
    {
        QJsonDocument doc =  QJsonDocument::fromJson(rep->readAll());
        if(doc.isNull())
            return;
        QJsonObject obj = doc.object();
        QString content = obj.value("output").toString();
        QString success = obj.value("success").toString();
        if(success == "true")
        {
            QString current = ui->func_code_edit->toPlainText();
            current.insert(last_pos,"\n"+content.replace("<unk>",""));
            ui->func_code_edit->setText(current);
        }
    }
    else
    {
        ui->output_response_edit->setText("Oops! Something went wrong");
    }
}


void fsIDE::on_signout_button_clicked()
{
    onlog_out();
    LoginPage *login = new LoginPage;
    close();
}


void fsIDE::on_checkBox_2_stateChanged(int arg1)
{
    if(ui->checkBox_2->isChecked())
        disable_AI = true;
    else
        disable_AI = false;
}

QString fsIDE::checkForErrors(QString code)
{
    QString error;
    QStringList lines = code.split(QString("\n"),Qt::KeepEmptyParts);
    QStringList defined_variables;
    defined_variables.append("Keyword");
    defined_variables.append("result");
    QString keywordNotGiven("Keyword function parameter not entered on IDE");
    QStringList functions = {"addJSONProp","append","dict","disableJavaScript","contains","createJSON","endsWith","findall","float","getPage","getAttribute","getImagesBy","getElementsByAttribute",
                            "getElementsByClass","getElementByID","getHref", "getElementsBy","getElementsByTag","getDictParameter","hasKeywords","enableJavaScript","int","least","joinByParameter","len","list",
                            "multiquote","postForm","postPage","readJSON","removeTag","split","strip","str","strReplace","sendTextPost","stripNonTextTags","stripTags"};
    QStringList functions_param_count = {"3","2","1","0","2","0","2","2","1","2","2","3","3",
                                         "2","2","2","2","2","2","2","0","1","2","3","1","0",
                                         "1","5","5","1","2","2","1","1","3","1","1","1"};
    if(ui->keyword_value_edit->text().trimmed().isEmpty())
    {
        return keywordNotGiven;
    }
    for(auto i{1}; i <= lines.length(); i++)
    {
        //check for variable initialization
        QString line_cleaned = lines.at(i-1).trimmed();
        if(line_cleaned == "")
            continue;
        line_cleaned.remove(QRegularExpression("\".*?\""));
        line_cleaned.replace("("," ( ");
        line_cleaned.replace(")", " ) ");
        line_cleaned.replace(";", " ;");
        line_cleaned.replace("+"," + ");
        line_cleaned.replace("["," [ ");
        line_cleaned.replace("]"," ] ");
        QStringList parts = line_cleaned.split(QRegularExpression("[\\s ]+"),Qt::SkipEmptyParts);
        if(parts.contains("="))
        {
            QRegularExpression is_num("(\\b[0-9]+\\b)");
            //is variable
            if(parts.at(0) == "var")
            {
                //new variables
                bool continues = true;
                if(functions.contains(parts.at(3)) && parts.at(4) == "(")
                {
                    continues = false;
                   defined_variables.append(parts.at(1));
                }
                if(continues && !is_num.match(parts.at(1)).hasMatch())
                {
                if(defined_variables.contains(parts.at(1)))
                {
                    return QString("Line " + QString::number(i) + " : " +"Variable " +  parts.at(1) + " is already defined.");
                }
                else
                {
                    defined_variables.append(parts.at(1));
                }
                }
            }

            if(!defined_variables.contains(parts.at(0)) && parts.at(0) != "var")
            {
                return QString("Line " + QString::number(i) + " : " +"Missing keyword var.");
            }
            for(auto j{2}; j < (parts.length() - 1) && !parts.at(2).contains("(");j++)
            {
                if(parts.at(j) == "=")
                    continue;
                if(functions.contains(parts.at(j)) && parts.at(j+1) == "(")
                {
                    j+=2;
                    continue;
                }
                if(!defined_variables.contains(parts.at(j)) && parts.at(j) != "+" && parts.at(j) == "*" && parts.at(j) == "/" && !is_num.match(parts.at(j)).hasMatch() && parts.at(j) != "," && parts.at(j) != ")" && parts.at(j) != "[" && parts.at(j) != "]")
                {
                     return QString("Line " + QString::number(i) + " : " +"Variable " +  parts.at(j) + " is not defined.");
                }
            }
        }
        if(parts.contains("+="))
        {
            QRegularExpression is_num("([0-9]+)");

            if(!defined_variables.contains(parts.at(0)))
            {
                 return QString("Line " + QString::number(i) + " : " +"Variable " +  parts.at(0) + " is not defined.");
            }
            for(auto j{2}; j < (parts.length() -1) && !parts.at(2).contains("(");j++)
            {
                if(parts.at(j) == "+=")
                {
                    continue;
                }
                if(functions.contains(parts.at(j)) && parts.at(j+1) == "(")
                {
                    continue;
                }
                if(!defined_variables.contains(parts.at(j)) && !is_num.match(parts.at(j)).hasMatch() && parts.at(j) != "+" && !defined_variables.contains(parts.at(j)))
                {
                     return QString("Line " + QString::number(i) + " : " +"Variable " +  parts.at(j) + " is not defined.");
                }
            }
        }
        if(parts.at(0) == "while" || parts.at(0) == "while(")
        {
            if(!defined_variables.contains("index"))
            {
                 return QString("Line " + QString::number(i) + " : " +"Variable index is not defined for while loop.");
            }
            QRegularExpression check_var("([a-zA-Z0-9_]+)\\)");
            QRegularExpressionMatch m = check_var.match(lines.at(i-1));
            QRegularExpression is_num("([0-9]+)\\)");
            if(m.hasMatch() && !is_num.match(lines.at(i-1)).hasMatch())
            {
                QString check_var_is = m.captured(1);
                if(!defined_variables.contains(check_var_is))
                {
                    return QString("Line " + QString::number(i) + " : " +"while loop terminating condition not defined.");
                }
            }
            else
            {
                return QString("Line " + QString::number(i) + " : Incorrect while loop");
            }
        }
        if(parts.at(0) == "for" || parts.at(0) == "for(")
        {
            QRegularExpression check_var("([a-zA-Z0-9_]+)\\)");
            QRegularExpressionMatch m = check_var.match(lines.at(i-1));
            QRegularExpression check_iter("\\(([a-zA-Z0-9_]+)");
            QRegularExpressionMatch m_i = check_iter.match(lines.at(i-1));
            if(!parts.contains("as"))
            {
                return QString("Line " + QString::number(i) + " : Incorrect for loop, as keyword missing");
            }
            if(m_i.hasMatch())
            {
                defined_variables.append(m_i.captured(1));
            }
            else
            {
                return QString("Line " + QString::number(i) + " : Incorrect for loop");
            }
            if(m.hasMatch())
            {
                QString check_var_is = m.captured(1);
                if(!defined_variables.contains(check_var_is))
                {
                    return QString("Line " + QString::number(i) + " : " +"for loop iterating variable not defined.");
                }
            }
            else
            {
                return QString("Line " + QString::number(i) + " : Incorrect for loop");
            }
        }
        if(line_cleaned.contains("(") && line_cleaned.contains(")"))
        {
            for(auto x{0}; x < functions.length(); x++)
            {
                if(lines.at(i-1).contains(functions.at(x) + "(") && functions.at(x) != "multiquote")
                {
                     QRegularExpression f_param("\\((.*)\\)");
                     QRegularExpressionMatch m_param = f_param.match(lines.at(i-1));
                     if(m_param.hasMatch())
                     {
                         QString p = m_param.captured(1);

                         int how_many = p.length() > 0 ? p.count(",") + 1 : 0;
                         if(how_many != functions_param_count.at(x).toInt())
                         {
                             return QString("Line " + QString::number(i) + " : " + "Incorrect number of parameters in " + functions.at(x));
                         }
                     }
                     else
                     {
                      return QString("Line " + QString::number(i) + " : Incomplete Function, " + functions.at(x));
                     }
                     break;
                }
            }
        }
        QString uncleaned_line = lines.at(i-1);

        uncleaned_line.replace(QRegularExpression("\\b\\d+\\b"),"$$$$A");
        uncleaned_line.replace(QRegularExpression("\"[^,]*[^\\\\,]?\""),"$$$$");
        uncleaned_line.replace(";", " ;");
        if(uncleaned_line == "{" || uncleaned_line == "}")
            continue;
        bool is_func = false;
        if(uncleaned_line.contains("(") & uncleaned_line.contains(")"))
            is_func = true;
        uncleaned_line.replace("\t"," ");
        uncleaned_line.replace("(", " ( ");
        uncleaned_line.replace(")", " )");
        uncleaned_line.replace(",",  " , ");
        uncleaned_line.replace("[", " [ ");
        uncleaned_line.replace("]"," ] ");
        uncleaned_line.replace("+=","$$$=");
        uncleaned_line.replace("+"," + ");
        uncleaned_line.replace("*"," * ");
        uncleaned_line.replace("/"," / ");
        uncleaned_line.replace("$$$=","+=");

        uncleaned_line.replace(";"," ;");
        QStringList parts_uncleaned = uncleaned_line.split(" ",Qt::SkipEmptyParts);
        if(uncleaned_line.count(";") > 1)
            return  QString("Line " + QString::number(i) + " : contains error with statement end ;");
        if(!uncleaned_line.trimmed().endsWith(";") && uncleaned_line.trimmed() != QString("{") && uncleaned_line.trimmed() != QString("}") && !parts_uncleaned.contains("for") && !parts_uncleaned.contains("while") && !parts_uncleaned.contains("if") && !parts_uncleaned.contains("elif") && !parts_uncleaned.contains("else"))
        {
            qDebug() << parts_uncleaned;
            return  QString("Line " + QString::number(i) + " : contains no statement end ;");
        }
        bool skip_line = false;
        QRegularExpression empty_func{"\\(\\s*?\\)"};
        skip_line = empty_func.match(uncleaned_line).hasMatch();
        if(parts_uncleaned.contains("multiquote") || parts_uncleaned.contains("for") || parts_uncleaned.contains("while") || parts_uncleaned.contains("if") || parts_uncleaned.contains("elif") || parts_uncleaned.contains("else"))
            skip_line = true;
        int start = is_func ? parts_uncleaned.indexOf("(") + 1 : parts_uncleaned.at(0) == "var" ? 3 : functions.contains(parts_uncleaned.at(0))? 2 : 2;
        for(auto y{start}; y < parts_uncleaned.length() - 1 && skip_line == false; y+= 2)
        {
            if(parts_uncleaned.at(y) == "[" || parts_uncleaned.at(y+1) == "[" || (parts_uncleaned.at(y) == "(" && parts_uncleaned.at(y+1) == ")"))
                break;
            if((defined_variables.contains(parts_uncleaned.at(y)) || parts_uncleaned.at(y) == "$$$$" || parts_uncleaned.at(y) == "$$$$A" || functions.contains(parts_uncleaned.at(y))) && (parts_uncleaned.at(y+1) == "+" || parts_uncleaned.at(y+1) == "/" || parts_uncleaned.at(y+1) == "*" || parts_uncleaned.at(y+1) == ";" || parts_uncleaned.at(y+1) == ")" || parts_uncleaned.at(y+1) == "," || parts_uncleaned.at(y+ 1) == ")"))
                continue;
            else
            {
                return  QString("Line " + QString::number(i) + " : Missing a + operand");
            }
        }

        for(auto iy{0}; iy < defined_variables.length(); iy++)
        {
            QString check_for = '\"' + defined_variables.at(iy) + '\"';
            if(lines.at(i-1).contains(check_for) && defined_variables.at(iy) != "result")
            {
                return QString("Line " + QString::number(i) + " : Literal (" + check_for +  ") conflicts with variable " + defined_variables.at(iy) + ". Please rename the variable!");
            }
        }
    }
    //Check for literals with the same value as variables
    return error;

}

void fsIDE::updateOutput()
{
    FoveaWebsiteInterface *interface_ = new FoveaWebsiteInterface;
    interface_->setCurrentRecieveMode(REQUEST_TYPE::APPCONTENT);
    interface_->setReceiver(&fsIDE::inputDebugOutput);
    interface_->setDestroyOnSuccess(true);
    interface_->requestDebugOutput(username,sessionkey);
}
