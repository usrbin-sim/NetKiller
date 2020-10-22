#include "get_info.h"

void get_iface_name(char * iface_name){
    // interface name
    FILE * stream = popen("getprop wifi.interface", "r");
    fgets(iface_name, 10, stream);
    pclose(stream);
    for(int i = 0; ; i++){
        if(iface_name[i] == '\n'){
            iface_name[i] = 0;
            break;
        }
    }
}

bool get_gw_ip(char * gw_ip){
    // gateway ip
    char output[100] = {0,};
    FILE * stream = popen("ip route get 8.8.8.8", "r");

    fgets(output, 100, stream);

    // Check Network
    if(strcmp(output, "") == 0){
        return false;
    }

    char *ptr = strtok(output, " ");
    int i = 0;
    while (ptr != nullptr){
        if(i == 2){
            strcpy(gw_ip, ptr);
            break;
        }
        i++;
        ptr = strtok(nullptr, " ");
    }
    return true;
}

void get_my_ip(char * my_ip){
    char output[100] = {0,};
    FILE * stream = popen("ip route get 8.8.8.8", "r");

    fgets(output, 100, stream);

    char *ptr = strtok(output, " ");
    int i = 0;
    while (ptr != nullptr){
        if(i == 6){
            strcpy(my_ip, ptr);
            return;
        }
        i++;
        ptr = strtok(nullptr, " ");
    }
}

void get_my_mac(uint8_t * mac, char * dev){
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s < 0) perror("socket fail");
    struct ifreq ifr;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    if(ioctl(s, SIOCGIFHWADDR, &ifr) < 0)
        perror("ioctl fail");
    unsigned char * tmp = reinterpret_cast<unsigned char *>(ifr.ifr_hwaddr.sa_data);

    memcpy(mac, tmp, sizeof(uint8_t)*6);

}
