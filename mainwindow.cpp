#include "mainwindow.h"
#include "ui_mainwindow.h"

std::vector<int> unicast_check;
int broadcast_check;

class MyScrollBar : public QScrollBar
{
public:
    MyScrollBar(QWidget * parent): QScrollBar(parent) {}

protected:
    QSize sizeHint() const override { return QSize(60, 0); }
    QSize minimumSizeHint() const override { return QSize(60, 0); }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    int ret = 0;
    timer = new QTimer;
    timer->start();
    timer->setInterval(1000);

    system("su -c \"/data/local/tmp/netkillerd&\"");

    // Check if server is running
    char tmp_buf[1024] = {0,};
    FILE *fp = popen("su -c \"ps | grep netkillerd\"", "rt");
    while(fgets(tmp_buf, 1024,fp)){
        if(strstr(tmp_buf, "netkillerd") != NULL){
            ret = 1;
            break;
        }
    }

    if(ret == 0){
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Error");
        MsgBox.setText("Server is not running");
        MsgBox.setStandardButtons(QMessageBox::Ok);
        MsgBox.setDefaultButton(QMessageBox::Ok);
        if ( MsgBox.exec() == QMessageBox::Ok ){
            exit(1);
        }
    }

    /* Get basic informations */
    /* 1. interface name */
    char iface_name[20] = { 0, };
    get_iface_name(iface_name);

    /* 2. My MAC */
    QString str_my_mac;
    uint8_t my_mac[6] = { 0, };
    get_my_mac(my_mac, iface_name);
    str_my_mac.sprintf("%02X:%02X:%02X:%02X:%02X:%02X", my_mac[0], my_mac[1], my_mac[2], my_mac[3], my_mac[4], my_mac[5]);

    /* 3. gateway IP */
    char str_gw_ip[30] = { 0, };
    uint32_t gw_ip;
    bool network_check = get_gw_ip(str_gw_ip);
    if(!network_check){
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Network unreachable!");
        MsgBox.setText("Check Wifi first.");
        MsgBox.setStandardButtons(QMessageBox::Ok);
        MsgBox.setDefaultButton(QMessageBox::Ok);
        if ( MsgBox.exec() == QMessageBox::Ok ){
            exit(1);
        }
    }
    gw_ip = inet_addr(str_gw_ip);

    /* 4. My IP */
    char str_my_ip[30] = { 0, };
    uint32_t my_ip;
    get_my_ip(str_my_ip);
    my_ip = inet_addr(str_my_ip);

    /* 5. Get Subnetmask */
    uint32_t subnet = get_subnet(iface_name);
    char str_subnet[30] {0, };
    sprintf(str_subnet, "%d.%d.%d.%d\n", (subnet)&0xFF, (subnet >> 8) & 0xFF, (subnet >> 16) & 0xFF, (subnet >> 24) & 0xFF);

    QStringList tableHeader;
    QStringList tableHeader2;
    ui->gwTable->setColumnCount(3);
    ui->devTable->setColumnCount(3);


#ifdef Q_OS_ANDROID
    ui->gwTable->setColumnWidth(0, 300);
    ui->gwTable->setColumnWidth(2, 100);
#else
    ui->gwTable->setColumnWidth(1, 180); // Set MAC size
    ui->gwTable->setColumnWidth(2, 50); // Set SELECT size
#endif // Q_OS_ANDROID

#ifdef Q_OS_ANDROID
    ui->devTable->setColumnWidth(1, 350);
    ui->devTable->setColumnWidth(2, 100);
#else
    ui->devTable->setColumnWidth(1, 180); // Set MAC size
    ui->devTable->setColumnWidth(2, 50); // Set SELECT size
#endif // Q_OS_ANDROID

#ifdef Q_OS_ANDROID
    ui->gwTable->setVerticalScrollBar(new MyScrollBar(ui->gwTable->verticalScrollBar()));
    ui->devTable->setVerticalScrollBar(new MyScrollBar(ui->devTable->verticalScrollBar()));
#endif // Q_OS_ANDROID

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont); // Use system fixed width font
    ui->gwTable->setFont(fixedFont);
    ui->devTable->setFont(fixedFont);

    ui->gwTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Always show scroll bar
    ui->devTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Always show scroll bar
    ui->gwTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable editing
    ui->devTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable editing

    tableHeader << "Interface" << "GW IP" << "";
    tableHeader2 << "MAC" << "IP" << "";

    ui->gwTable->setHorizontalHeaderLabels(tableHeader);
    ui->gwTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->gwTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->gwTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);

    ui->gwTable->verticalHeader()->setDefaultSectionSize(80); // height
    ui->gwTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);

    ui->gwTable->insertRow(ui->gwTable->rowCount());
    QTableWidgetItem * iface_name_item = new QTableWidgetItem(iface_name);
    iface_name_item->setTextAlignment(Qt::AlignCenter);
    QTableWidgetItem * gw_ip_item = new QTableWidgetItem(str_gw_ip);
    gw_ip_item->setTextAlignment(Qt::AlignCenter);
    ui->gwTable->setItem(0, 0, iface_name_item);
    ui->gwTable->setItem(0, 1, gw_ip_item);

    btn_attack = new QPushButton(this);

    btn_attack->setIcon(QIcon(":/images/start.png"));
    btn_attack->setIconSize(QSize(70,70));
    is_broad = false;

    QObject::connect(btn_attack, &QPushButton::clicked, this, &MainWindow::braodAttack);
    ui->gwTable->setCellWidget(0, 2,(QWidget*)btn_attack);

    ui->devTable->setHorizontalHeaderLabels(tableHeader2);
    ui->devTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->devTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->devTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);

    ui->devTable->verticalHeader()->setDefaultSectionSize(80); // height
    ui->devTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);

    int server_port = 1234;
    if(!connect_sock(&client_sock, server_port)){

        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Socket failed");
        MsgBox.setText("Socket creation failed. plz restart.");
        MsgBox.setStandardButtons(QMessageBox::Ok);
        MsgBox.setDefaultButton(QMessageBox::Ok);
        if ( MsgBox.exec() == QMessageBox::Ok ){
            exit(1);
        }
    }

    thread = new Thread(client_sock);
    QObject::connect(thread, &Thread::captured, this, &MainWindow::processCaptured);

    thread->start();

    /* Send basic informations to Netkiller daemon */
    memset(buf, 0x00, BUF_SIZE);
    memcpy(buf, "1", 1);
    send_data(client_sock, buf);

    /* Scan devices */
    memset(buf, 0x00, BUF_SIZE);
    memcpy(buf, "3", 1);
    send_data(client_sock, buf);

}

MainWindow::~MainWindow()
{
    system("su -c \"killall -9 netkillerd\"");
    delete ui;
}

void MainWindow::braodAttack(){
    QPushButton * pb = (QPushButton *)sender();
    char sdata[BUF_SIZE];
    memset(sdata, 0x00, BUF_SIZE);

    if (is_broad == false){
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Attack Start");
        MsgBox.setText("broadcast attack started!");
        MsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        MsgBox.setDefaultButton(QMessageBox::Ok);
        if ( MsgBox.exec() == QMessageBox::Ok ){
            broadcast_check = true;
            pb->setIcon(QIcon(":/images/stop.png"));
            pb->setIconSize(QSize(70,70));

            memcpy(sdata, "2", 1);
            send_data(client_sock, sdata);
            is_broad = true;
            for(auto it = unicast_btn_list.begin(); it != unicast_btn_list.end(); it++)
            {
                (*it)->setEnabled(0);
            }
        }
    } else {
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Attack Stop");
        MsgBox.setText("broadcast attack stop.");
        MsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        MsgBox.setDefaultButton(QMessageBox::Ok);
        if ( MsgBox.exec() == QMessageBox::Ok ){
            pb->setIcon(QIcon(":/images/start.png"));
            pb->setIconSize(QSize(70,70));
            broadcast_check = false;
            memcpy(sdata, "5", 1);
            send_data(client_sock, sdata);
            is_broad = false;
            for(auto it = unicast_btn_list.begin(); it != unicast_btn_list.end(); it++)
            {
                (*it)->setEnabled(1);
            }
        }
    }
}

void MainWindow::unicastAttack(){
    char sdata[BUF_SIZE];
    QPushButton *pb = qobject_cast<QPushButton *>(QObject::sender());
    int index = pb->property("my_key").toInt();

    memset(sdata, 0x00, BUF_SIZE);
    if (is_unicast[index] == false){
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Unicast attack start");
        MsgBox.setText(ui->devTable->item(index, 0)->text());
        MsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        MsgBox.setDefaultButton(QMessageBox::Ok);
        if ( MsgBox.exec() == QMessageBox::Ok ){
            unicast_check.push_back(1);

            pb->setIcon(QIcon(":/images/stop.png"));
            pb->setIconSize(QSize(70,70));

            sdata[0] = '4';
            sdata[1] = '\t';
            strcat(sdata, ui->devTable->item(index, 0)->text().toStdString().c_str());
            sdata[strlen(sdata)] = '\t';
            strcat(sdata, ui->devTable->item(index, 1)->text().toStdString().c_str());
            sdata[strlen(sdata)] = '\t';
            send_data(client_sock, sdata);
            btn_attack->setEnabled(0);
            is_unicast[index] = true;
        }
    } else {
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Unicast attack stop");
        MsgBox.setText(ui->devTable->item(index, 0)->text());
        MsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        MsgBox.setDefaultButton(QMessageBox::Ok);
        if ( MsgBox.exec() == QMessageBox::Ok ){
            pb->setIcon(QIcon(":/images/start.png"));
            pb->setIconSize(QSize(70,70));

            unicast_check.pop_back();
            sdata[0] = '6';
            sdata[1] ='\t';
            strcat(sdata, ui->devTable->item(index, 0)->text().toStdString().c_str());
            sdata[strlen(sdata)] = '\t';
            send_data(client_sock, sdata);
            is_unicast[index] = false;
        }
        if(unicast_check.empty()){
            btn_attack->setEnabled(1);
        }
    }

}

void MainWindow::processCaptured(char* data)
{
    QString temp = QString(data);
    QStringList info = temp.split("\t");

    if(info[0] == '3'){
        ui->devTable->insertRow(ui->devTable->rowCount());
        int index = ui->devTable->rowCount() - 1;

        QTableWidgetItem * mac = new QTableWidgetItem(info[1]);
        mac->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem * ip = new QTableWidgetItem(info[2]);
        ip->setTextAlignment(Qt::AlignCenter);
        ui->devTable->setItem(index, 0, mac);
        ui->devTable->setItem(index, 1, ip);
        QPushButton* btn_attack = new QPushButton();

        btn_attack->setIcon(QIcon(":/images/start.png"));
        btn_attack->setIconSize(QSize(70,70));

        btn_attack->setProperty("my_key", index);
        unicast_btn_list.append(btn_attack);
        is_unicast.push_back(false);

        ui->devTable->setCellWidget(index, 2, (QWidget*)btn_attack);
        QObject::connect(btn_attack, &QPushButton::clicked, this, &MainWindow::unicastAttack);
    }


}
