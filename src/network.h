#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

typedef struct Network Network;
struct Network
{
    WSADATA wsa_data;
};

Network* 
CreateNetwork()
{
    Network* network;
    network = (Network*)calloc(1, sizeof(*network));
    int result = WSAStartup(MAKEWORD(2,2), &network->wsa_data);
    if(result != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d", result);
        return NULL;
    }
    return network;
}
