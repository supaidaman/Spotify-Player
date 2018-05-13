#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtNetworkAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//Cria tela e faz setup das chaves to spotify. Seta seleção das grids para linhas
    ui->setupUi(this);
    ui->tableWidget->setSelectionBehavior(QTableView::SelectRows);
    ui->tableWidget_2->setSelectionBehavior(QTableView::SelectRows);
    auto replyHandler = new QOAuthHttpServerReplyHandler(8080, NULL);
    musicPlayer = new QMediaPlayer();
     playlist = new QMediaPlaylist();
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
}

MainWindow::~MainWindow()
{

    delete ui;
}



//Método que recebe resposta do spotify.
void MainWindow::authorizationStatusChanged(QAbstractOAuth::Status status)
{
    QString s;
    if (status == QAbstractOAuth::Status::Granted){
        s = "Permissão Concedida";
        ui->searchButton->setEnabled(true);
    }
    if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
        s = "temp credentials";
        //oauth2.refreshAccessToken();
    }

    ui->label->setText("Status da Autorização: " + s +  "\n");

}

//Dar Permissão
void MainWindow::on_permissionButton_clicked()
{
     spotify.grant();
}


//Pega nome do Usuário. Usado como teste. Botão desabilitado
void MainWindow::on_retrieveUserInfoButton_clicked()
{
    QUrl u ("https://api.spotify.com/v1/me");

    auto reply = spotify.get(u);

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
         qWarning () << "Erro Encontrado";
            return;
        }

        ui->label->setText("Informações do Usuário Recuperadas");
        const auto data = reply->readAll();

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        userName = root.value("id").toString();


        reply->deleteLater();
    });
}

//Recupera Playlist do usuário. Usado como teste.
void MainWindow::on_recoverUserPlaylist_clicked()
{
    if (userName.length() == 0) return;


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

//Procura Música pelo nome digitado. Procura somente "tracks"(músicas)

//Adiciona músicas encontradas na lista (tableWidget).

void MainWindow::on_searchButton_clicked()
{
    if(ui->searchLine->text()== NULL || ui->searchLine->text()=="")
    {
        QMessageBox::information(this, tr("A busca está vazia!"),
        "Digite Algo Para Buscar");
        return;
    }
    ui->tableWidget->setRowCount(0);
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
            auto previewUrl = itemMap["preview_url"];

            ui->tableWidget->insertRow ( ui->tableWidget->rowCount() );
            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     0,
                                     new QTableWidgetItem(name.toString()));

            ui->tableWidget->setItem   ( ui->tableWidget->rowCount()-1,
                                     1,
                                     new QTableWidgetItem(previewUrl.toString()));



        }


        reply->deleteLater();
    });
}

//Métodos para salvar/carregar playlist
void MainWindow::on_actionSalvar_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
         tr("Salvar Playlist"), "",
         tr("Arquivo de Texto (*.txt);;Todos os Arquivos (*)"));

    if (fileName.isEmpty())
          return;
      else {
          QFile file(fileName);
          if (!file.open(QIODevice::WriteOnly)) {
              QMessageBox::information(this, tr("Não é possível abrir o arquivo"),
                  file.errorString());
              return;
          }

          QTextStream out(&file);
          for(int i =0; i< ui->tableWidget_2->rowCount();++i)
          {
              auto itemURL = ui->tableWidget_2->item(i,1);
              auto itemName = ui->tableWidget_2->item(i,0);
              out << QString(itemName->text() + ";" + itemURL->text()) << "\r\n"; //windows line terminator
          }
      }
}

void MainWindow::on_actionCarregar_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
           tr("Abrir Playlist"), "",
           tr("Arquivo de Texto (*.txt);;Todos os Arquivos (*)"));
    if (fileName.isEmpty())
           return;
       else {

           QFile file(fileName);

           if (!file.open(QIODevice::ReadOnly)) {
               QMessageBox::information(this, tr("Não foi possível abrir o arquivo"),
                   file.errorString());
               return;
           }

           QTextStream in(&file);
           ui->tableWidget_2->setRowCount(0);
           while (!in.atEnd())
              {
                 QString line = in.readLine();
                 QStringList lstLine = line.split(";");
                 ui->tableWidget_2->insertRow ( ui->tableWidget_2->rowCount() );
                 ui->tableWidget_2->setItem   ( ui->tableWidget_2->rowCount()-1,
                                          0,
                                          new QTableWidgetItem(lstLine[0]));
                 ui->tableWidget_2->setItem   ( ui->tableWidget_2->rowCount()-1,
                                          1,
                                          new QTableWidgetItem(lstLine[1]));


              }
       }
}

void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{

    auto itemRow = item->row();


    int currentIndex = 0;
    ui->tableWidget_2->insertRow ( ui->tableWidget_2->rowCount() );
    for(int i =0; i< ui->tableWidget->columnCount();++i)
    {
        auto itemSelected = ui->tableWidget->item(itemRow,i);
        ui->tableWidget_2->setItem   ( ui->tableWidget_2->rowCount()-1,
                                   currentIndex,
                                  new QTableWidgetItem(itemSelected->text()));
       currentIndex++;
       if(currentIndex>1)
           currentIndex=0;

    }

}

//tocando preview
void MainWindow::on_playButton_clicked()
{
//criar outros botões// colocar campo na playlist com nome do artista.
    if(musicPlayer->state()!= QMediaPlayer::PausedState)
    {
      if(ui->tableWidget_2->rowCount()==0)
      {
        QMessageBox::information(this, tr("A playlist está vazia!"),
        "Insira músicas na playlist para começar");
        return;
      }
      playlist = new QMediaPlaylist();
      for(int i =0; i< ui->tableWidget_2->rowCount();++i)
      {
          auto itemSelected = ui->tableWidget_2->item(i,1);
             playlist->addMedia(QUrl(itemSelected->text()));
      }

      musicPlayer->setPlaylist(playlist);
    }
      musicPlayer->play();


}

void MainWindow::on_pauseButton_clicked()
{
    if(ui->tableWidget_2->rowCount()==0)
    {
      QMessageBox::information(this, tr("A playlist está vazia!"),
      "Insira músicas na playlist para começar");
      return;
    }
    musicPlayer->pause();
}

void MainWindow::on_stopButton_clicked()
{
    if(ui->tableWidget_2->rowCount()==0)
    {
      QMessageBox::information(this, tr("A playlist está vazia!"),
      "Insira músicas na playlist para começar");
      return;
    }
    musicPlayer->stop();
}

void MainWindow::on_removeButton_clicked()
{
    if(ui->tableWidget_2->rowCount()==0)
    {
      QMessageBox::information(this, tr("A playlist está vazia!"),
      "Insira músicas na playlist para começar");
      return;
    }

    auto item = ui->tableWidget_2->selectedItems();

    //Remove por linha
    for(int i =0;i<= item.count()/2;i+=2)
    {
        ui->tableWidget_2->removeRow(item[i]->row());
    }
}
