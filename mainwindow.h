#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "get_info.h"
#include "socket.h"
#include "thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int client_sock;
    Thread * thread;
    char data[BUF_SIZE];
    char buf[BUF_SIZE];
    QList<QPushButton *> unicast_btn_list;
    QPushButton* btn_attack;

public slots:
    void braodAttack();
    void unicastAttack();
    void processCaptured(char* data);

private:
    Ui::MainWindow *ui;
    QTimer *timer;

signals:
    void addDev();
};
#endif // MAINWINDOW_H
