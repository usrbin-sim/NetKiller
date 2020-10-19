#pragma once

#include <QThread>
#include <socket.h>
#include "socket.h"

struct Thread : QThread {
    Q_OBJECT

public:
    Thread(int sock) {
        client_sock = sock;
    }

    ~Thread() override {
    }

    bool active_{false};
    int client_sock;

protected:
    bool open();
    bool close();

protected:
    void run() override;

public:
signals:
    void captured(char * data);
};
