#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "network.h"

B8
CreateSocket(Server* server, I32 port)
{
    server->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server->socket==INVALID_SOCKET)
    {
        fprintf(stderr, "Error creating socket: %ld\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    struct sockaddr_in server_addr;   
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    I32 result = bind(server->socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(result == SOCKET_ERROR)
    {
        fprintf(stderr, "Bind failed: %ld\n", WSAGetLastError());
        closesocket(server->socket);
        WSACleanup();
        return false;
    }
    
    return true;
}

int StartListening(Server* server)
{
    if(listen(server->socket, SOMAXCONN) == SOCKET_ERROR)
    {
        fprintf(stderr, "Listening failed: %ld\n", WSAGetLastError());
        closesocket(server->socket);
        WSACleanup();
        return false;
    }

    printf("Now starting to listen, please connect.\n");

    SOCKET client_socket = accept(server->socket, NULL, NULL);
    if(client_socket == INVALID_SOCKET)
    {
        fprintf(stderr, "Accept failed: %ld\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return false;
    }

    return true;

#define SERVER_BUF_LEN 512
    char buf[SERVER_BUF_LEN];

    int bytes_received = recv(client_socket, buf, SERVER_BUF_LEN, 0);
    if(bytes_received > 0)
    {
        printf("Received %d bytes", bytes_received);
        send(client_socket, buf, bytes_received, 0);
    }
    else if(bytes_received==0)
    {
        printf("Closing connection...\n");
    }
    else
    {
        fprintf(stderr, "Connection failed: %ld", WSAGetLastError());
    }

    closesocket(client_socket);
    return 0;
}

void CloseServer(Server* server)
{
    closesocket(server->socket);
    WSACleanup();
}

Server* 
CreateServer()
{
    printf("Starting server\n");
    Server* server;
    server = (Server*)calloc(1, sizeof(*server));
    I32 result = WSAStartup(MAKEWORD(2,2), &server->wsa_data);
    if(result != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d", result);
        return NULL;
    }

    CreateSocket(server, 7070);

    return server;
}

int main(int argc, char** argv)
{
    Server* server = CreateServer();
    StartListening(server);
    return 0;
}
