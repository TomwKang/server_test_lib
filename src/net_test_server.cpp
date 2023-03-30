#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <functional>
#include "net_test.hpp"

void UDPServer()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int sockfd = -1;
    int udp = 0;
    SocketSetup(udp, sockfd, addr, addr_len, port_out);
    SocketBind(sockfd, addr, addr_len, port_out);
    SocketRecv(udp, sockfd);

    // boost::thread UDPServerRecv(SocketRecv,udp,sockfd);
    // UDPServerRecv.join();

    close(sockfd);
}

void TCPServer()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int sockfd = -1;
    int tcp = 1;
    SocketSetup(tcp, sockfd, addr, addr_len, port_tcp);
    SocketBind(sockfd, addr, addr_len, port_out);
    SocketListen(sockfd, port_tcp);
    SocketAccept(sockfd, addr, addr_len, port_tcp);
    SocketRecv(tcp, sockfd);

    // boost::thread TCPServerRecv(SocketRecv,tcp,sockfd);
    // TCPServerRecv.join();

    close(sockfd);
}

int main()
{
    int tcp_or_udp = 1;
    std::cout << "Please input 0 for UDP, 1 for TCP: ";
    std::cin >> tcp_or_udp;
    if (tcp_or_udp == 0)
    {
        UDPServer();
    }
    else
    {
        TCPServer();
    }
    return 0;
}