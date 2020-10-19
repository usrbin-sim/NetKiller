#include "socket.h"

bool connect_sock(int * client_sock, int server_port)
{
    bool ret = true;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    if ((*client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) // make client socket
        exit(1);

    memset(&client_addr, 0x00, addr_size);

    memset(&server_addr, 0x00, sizeof(server_addr));

    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (connect(*client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // connect server socket
    {
        ret = false;
    }

    getsockname(*client_sock, (struct sockaddr *)&client_addr, &addr_size); // get client port

    return ret;
}

bool send_data(int client_sock, char *data)
{
    char result[BUF_SIZE] = {0};
    unsigned int data_length = strlen(data);
    result[0] = (data_length >> 8) & 0xFF;
    result[1] = data_length & 0xFF;
    memcpy(result + 2, data, strlen(data));

    if (write(client_sock, result, strlen(data) + 2) <= 0)
    {
        return false;
    }
    return true;
}

bool recv_data(int client_sock, char *data)
{
    unsigned char buf[BUF_SIZE] = {0};
    char result[BUF_SIZE] = {0};
    unsigned int data_length = 0;

    if (read(client_sock, buf, 2) < 0)
    {
        return false;
    }

    data_length = (buf[0] << 8) + buf[1];

    if (read(client_sock, result, data_length) <= 0)
    {
        return false;
    }

    result[data_length] = '\0';

    memcpy(data, result, sizeof(result));

    return true;
}
