#include "thread.h"

bool Thread::open() {
    active_ = true;
    return true;
}

bool Thread::close() {
    active_ = false;
    return true;
}

void Thread::run() {

    if (!open()) return;

    fd_set readfds;
    int fd = client_sock;
    int state;
    struct    timeval tv;

    char data[1024] = {0};
    char buf[1024] = {0};

    while (active_)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        if ((state = select(fd + 1, &readfds, 0, 0, &tv)) == -1)
        {
            perror("select() error");
            exit(0);
            break;
        }
        if (state == 0)
            continue;

        memset(data, 0x00, 1024);
        recv_data(client_sock, data);
        emit captured(data);
    }

    memset(buf, 0x00, 1024);

    send_data(client_sock, buf);

    close();

}
