#include "mainwindow.h"
#include <QApplication>

#include <QSslSocket>
#include <QDebug>

int main(int argc, char *argv[])
{
   //Verifica Suporte A SSL
    if (!QSslSocket::supportsSsl()) {
        qWarning () << "Sem Suporte A SLL";
        exit (1);
    }
    qDebug () << QSslSocket::sslLibraryVersionString();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
