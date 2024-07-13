#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

typedef struct Network Network;
struct Network
{
    WSADATA wsa_data;
    SOCKET socket;
};

int 
CreateSocket(Network* network, I32 port)
{
    network->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(network->socket==INVALID_SOCKET)
    {
        fprintf(stderr, "Error creating socket: %ld", WSAGetLastError());
        WSACleanup();
        return 0;
    }

    struct sockaddr_in server_addr;   
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    I32 result = bind(network->socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(result == SOCKET_ERROR)
    {
        fprintf(stderr, "Bind failed: %ld\n", WSAGetLastError());
        closesocket(network->socket);
        WSACleanup();
        return 0;
    }
    
    return 1;
}

Network* 
CreateNetwork()
{
    Network* network;
    network = (Network*)calloc(1, sizeof(*network));
    I32 result = WSAStartup(MAKEWORD(2,2), &network->wsa_data);
    if(result != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d", result);
        return NULL;
    }
    return network;
}
