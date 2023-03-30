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
using namespace std;
#include "net_test.hpp"

long long file_size = 10 * MB_SIZE;
// std::string address = "127.0.0.1";
// addr.sin_addr.s_addr = htonl(inet_addr(address.c_str()));
void UDPClient()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int sockfd = -1;
    char *send_buffer = new char[file_size];
    memset(send_buffer, 'D', file_size);
    int udp = 0;
    SocketSetup(udp, sockfd, addr, addr_len, port_in);
    SocketBind(sockfd, addr, addr_len, port_in);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_out);
    addr.sin_addr.s_addr = htonl(inet_addr("172.20.30.92"));
    SocketSend(udp, sockfd, addr, addr_len, port_out, send_buffer);
    // boost::thread UDPClientSend(SocketSend,udp,sockfd, addr, addr_len, port_out, send_buffer);
    // UDPClientSend.join();

    close(sockfd);
    delete[] send_buffer;
    send_buffer = nullptr;
}
void TCPClient(int times = 10)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int tcp_sockfd = -1;
    char *send_buffer = new char[file_size];
    memset(send_buffer, 'T', file_size);
    int tcp = 1;

    SocketSetup(tcp, tcp_sockfd, addr, addr_len, port_tcp);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_tcp);
    string address = "172.20.30.92";
    addr.sin_addr.s_addr = inet_addr(address.c_str());
    SocketConnect(tcp_sockfd, addr, addr_len, port_tcp);

    std::cout << "\n=============TimeCounter=============" << std::endl;
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < times; i++)
    {
        SocketSend(tcp, tcp_sockfd, addr, addr_len, port_tcp, send_buffer);
    }

    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算耗时（毫秒）
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // 输出结果
    std::cout << "All Time: " << duration << " ms" << std::endl;
    std::cout << "Average Time: " << static_cast<double>(duration) / times << " ms" << std::endl;
    std::cout << "Average Speed: " << 2 * static_cast<double>(file_size / MB_SIZE) * 1000 / (static_cast<double>(duration) / times) << " MB/s" << std::endl;
    std::cout << "=============TimeCounter=============\n"
              << std::endl;
    // boost::thread TCPClientSend(SocketSend, tcp, tcp_sockfd, addr, addr_len, port_out, send_buffer);
    // TCPClientSend.join();

    close(tcp_sockfd);
    delete[] send_buffer;
    send_buffer = nullptr;
}

int main(int argc, char const *argv[])
{
    int tcp_or_udp = 1;
    std::cout << "Please input 0 for UDP, 1 for TCP: ";
    std::cin >> tcp_or_udp;
    if (tcp_or_udp == 0)
    {
        TimeCounter(UDPClient, 10, TestType::UDP, file_size);
    }
    else
    {
        TCPClient(10);
    }
    return 0;
}