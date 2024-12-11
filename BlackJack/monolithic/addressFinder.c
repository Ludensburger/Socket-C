#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma comment(lib, "iphlpapi.lib")

// gcc addressFinder.c -liphlpapi -o addressFinder
// addressFinder.exe

unsigned int ipToInt(const char *ip) {
    unsigned int a, b, c, d;
    sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);
    return (a << 24) | (b << 16) | (c << 8) | d;
}

void intToIp(unsigned int ip, char *buffer) {
    sprintf(buffer, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
}

void getLocalIp(char *buffer) {
    PIP_ADAPTER_INFO adapterInfo;
    DWORD bufferSize = sizeof(IP_ADAPTER_INFO);
    adapterInfo = (IP_ADAPTER_INFO *)malloc(bufferSize);

    if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
        free(adapterInfo);
        adapterInfo = (IP_ADAPTER_INFO *)malloc(bufferSize);
    }

    if (GetAdaptersInfo(adapterInfo, &bufferSize) == NO_ERROR) {
        PIP_ADAPTER_INFO adapter = adapterInfo;
        while (adapter) {
            if (adapter->Type == MIB_IF_TYPE_ETHERNET && adapter->IpAddressList.IpAddress.String[0] != '0') {
                strcpy(buffer, adapter->IpAddressList.IpAddress.String);
                break;
            }
            adapter = adapter->Next;
        }
    }

    free(adapterInfo);
}

void calculateAddresses(char *ip, char *subnet) {
    unsigned int ip_addr = ipToInt(ip);
    unsigned int subnet_mask = ipToInt(subnet);
    unsigned int network_addr = ip_addr & subnet_mask;
    unsigned int broadcast_addr = network_addr | ~subnet_mask;

    char network_str[16], broadcast_str[16];
    intToIp(network_addr, network_str);
    intToIp(broadcast_addr, broadcast_str);

    printf("\n");

    printf("IP Address: %s\n", ip);
    printf("Subnet Mask: %s\n", subnet);
    printf("----------------------------------------\n");

    printf("Network Address: %s\n", network_str);
    printf("Broadcast Address: %s\n", broadcast_str);
    printf("----------------------------------------\n");

    unsigned int host_count = ~subnet_mask - 1;
    printf("Number of Usable Hosts: %u\n", host_count);

    unsigned int start_addr = network_addr + 1;
    unsigned int end_addr = broadcast_addr - 1;

    char start_str[16], end_str[16];
    intToIp(start_addr, start_str);
    intToIp(end_addr, end_str);

    printf("Usable Host IP Range: %s - %s\n", start_str, end_str);

    char local_ip[16];
    getLocalIp(local_ip);

    printf("Current Machine's IPv4 Address: %s\n", local_ip);

    char host_str[16];

    strcpy(host_str, local_ip);
    if (strcmp(host_str, local_ip) == 0) {
        printf("Machine is currently using a usable host address.\n");
    } else {
        printf("Machine is not using a usable host address.\n");
    }

    printf("----------------------------------------\n");
    printf("\n");

    printf("Number of Usable Host Address: %d\n", host_count);
    printf("\n");
    printf("Usable Host Addresses:\n");

    for (unsigned int i = start_addr; i <= end_addr; i++) {

        intToIp(i, host_str);

        if (strcmp(host_str, local_ip) == 0) {
            printf("%s *\n", host_str);
        } else {
            printf("%s\n", host_str);
        }
    }
}

int main() {
    char ip[16], subnet[16];
    system("cls");
    printf("\033[H\033[J"); // ANSI escape code to clear the terminal

    // switch case 1, 2 and 3
    // if 1 then manually input ip and subnet
    // if 2 then use the declaration above
    // if 3 then use ip and subnet from the main computer and just ask for subnet

    int op;

    printf("1. Manually Enter IP and Subnet Mask\n");
    printf("2. Use Default IP and Subnet Mask\n");
    printf("3. Use Local IP and Enter Subnet Mask\n");
    printf("\n");

    printf("Op: ");
    scanf("%d", &op);

    switch (op) {
    case 1:
        printf("Enter IP Address: ");
        scanf("%15s", ip);
        printf("Enter Subnet Mask: ");
        scanf("%15s", subnet);

        break;

    case 2:
        strcpy(ip, "192.168.0.1");
        strcpy(subnet, "255.255.255.0");

        break;

    case 3:
        getLocalIp(ip);
        printf("Enter Subnet Mask: ");
        scanf("%15s", subnet);

        break;

    case -1:
        printf("Exiting...\n");
        break;

    default:
        printf("Invalid option.\n");
        break;
    }

    calculateAddresses(ip, subnet);

    return 0;
}