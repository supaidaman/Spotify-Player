#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtNetworkAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    auto replyHandler = new QOAuthHttpServerReplyHandler(8080, NULL);

    spotify.setReplyHandler(replyHandler);
    spotify.setAuthorizationUrl(QUrl("https://accounts.spotify.com/authorize"));
    spotify.setAccessTokenUrl(QUrl("https://accounts.spotify.com/api/token"));
    spotify.setClientIdentifier("59aea641a6cd4e179935ccc0a27757ed");
    spotify.setClientIdentifierSharedKey("05384087670b46dd840b7c49ee5e1d2c");
    spotify.setScope("user-read-private user-top-read playlist-read-private playlist-modify-public playlist-modify-private");
    connect(&spotify, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
             &QDesktopServices::openUrl);

    connect(&spotify, &QOAuth2AuthorizationCodeFlow::statusChanged,
            this, &MainWindow::authorizationStatusChanged);

//    connect(&spotify, &QOAuth2AuthorizationCodeFlow::granted,
//            this, &MainWindow::grantPermission);


}

MainWindow::~MainWindow()
{

    delete ui;
}

void MainWindow::grantPermission()
{
     spotify.grant();
}

void MainWindow::authorizationStatusChanged(QAbstractOAuth::Status status)
{
    QString s;
    if (status == QAbstractOAuth::Status::Granted)
        s = "granted";

    if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
        s = "temp credentials";
        //oauth2.refreshAccessToken();
    }

    ui->label->setText("Status da Autorização: " + s +  "\n");

}

void MainWindow::grantedPermission()
{

}


void MainWindow::getUserData()
{
    QUrl u ("https://api.spotify.com/v1/me");

    auto reply = spotify.get(u);

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
         qWarning () << "Erro Encontrado";
            return;
        }
        const auto data = reply->readAll();


        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        userName = root.value("id").toString();



        reply->deleteLater();
    });
}

void MainWindow::on_permissionButton_clicked()
{
     spotify.grant();
}

void MainWindow::on_retrieveUserInfoButton_clicked()
{
    QUrl u ("https://api.spotify.com/v1/me");

    auto reply = spotify.get(u);

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
         qWarning () << "Erro Encontrado";
            return;
        }

        ui->label->setText("AEEEE");
        const auto data = reply->readAll();

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        userName = root.value("id").toString();


        reply->deleteLater();
    });
}

void MainWindow::on_recoverUserPlaylist_clicked()
{
    if (userName.length() == 0) return;

//    ui->tableView->appendPlainText("Loading Playlists ...");

    QUrl u ("https://api.spotify.com/v1/users/" + userName + "/playlists");

    auto reply = spotify.get(u);

    connect (reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {

            return;
        }

        const auto data = reply->readAll();

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        QString rootValue = root.value("id").toString();

        QVariantMap root_map = root.toVariantMap();
        QVariantMap stat_map = root_map["href"].toMap();
        QVariantList item_list = root_map["items"].toList();


        for(int i =0; i< item_list.count();i++)
        {
            QVariantMap itemMap = item_list[i].toMap();
            auto ID  = itemMap["id"];
            auto Name = itemMap["name"]; // montar lista de playlists baseado nisto
            ui->tableWidget->insertRow ( ui->tableWidget->rowCount() );
            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     0,
                                     new QTableWidgetItem(Name.toString()));
            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     1,
                                     new QTableWidgetItem(ID.toString()));



        }



        reply->deleteLater();
    });
}



void MainWindow::on_searchButton_clicked()
{
    QUrl u ("https://api.spotify.com/v1/search?q="+ui->searchLine->text() +"&type=track");
    auto reply = spotify.get(u);

    connect (reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning () << reply->errorString();
            return;
        }

        const auto data = reply->readAll();

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        QString rootValue = root.value("id").toString();

        QVariantMap root_map = root.toVariantMap();
        QVariantMap tracks_map = root_map["tracks"].toMap();
        QVariantList track_list = tracks_map["items"].toList();

        for(int i =0; i< track_list.count();i++)
        {
            QVariantMap itemMap = track_list[i].toMap();
            auto ID  = itemMap["id"];
            auto name = itemMap["name"]; // montar lista de playlists baseado nisto
            auto href = itemMap["href"];

            ui->tableWidget->insertRow ( ui->tableWidget->rowCount() );
            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     0,
                                     new QTableWidgetItem(name.toString()));
            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     1,
                                     new QTableWidgetItem(ID.toString()));
            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     1,
                                     new QTableWidgetItem(ID.toString()));
            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     2,
                                     new QTableWidgetItem(href.toString()));



        }


        reply->deleteLater();
    });
}
