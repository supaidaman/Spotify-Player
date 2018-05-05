#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QOAuth2AuthorizationCodeFlow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void grantPermission();
    void authorizationStatusChanged (QAbstractOAuth::Status status);
    void grantedPermission();
    void getUserData();

private slots:
    void on_permissionButton_clicked();

    void on_retrieveUserInfoButton_clicked();

    void on_recoverUserPlaylist_clicked();

    void on_searchButton_clicked();

private:
    Ui::MainWindow *ui;
    QOAuth2AuthorizationCodeFlow spotify;
    bool isGranted;
    QString userName;

};

#endif // MAINWINDOW_H
