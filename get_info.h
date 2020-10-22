#ifndef GET_INFO_H
#define GET_INFO_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

void get_iface_name(char * iface_name);
bool get_gw_ip(char * gw_ip);
void get_my_ip(char * my_ip);
void get_my_mac(uint8_t * mac, char * dev);

#endif // GET_INFO_H
