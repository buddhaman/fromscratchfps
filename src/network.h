#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

typedef struct Server Server;
struct Server
{
    WSADATA wsa_data;
    SOCKET socket;
};
