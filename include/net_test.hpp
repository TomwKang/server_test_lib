#ifndef NETWORK_TEST_TCP_TEST_HPP
#define NETWORK_TEST_TCP_TEST_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>
#include <limits.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <net/if.h>
#include <sys/ioctl.h>
// #include <boost/thread.hpp>

int port_in = 10023, port_out = 10024, port_tcp = 11451;
const long long KB_SIZE = 1024;
const long long MB_SIZE = 1024 * 1024;
const long long GB_SIZE = 1024 * 1024 * 1024;

enum class TestType
{
    TCP,
    UDP,
    DickIO,
    Mem,
};

void SocketSetup(int tcp_or_udp, int &sockfd, struct sockaddr_in &addr, socklen_t &addr_len, int port)
{
    if (tcp_or_udp == 0)
        sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP
    else
        sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP
    if (-1 == sockfd)
    {
        std::cout << "Failed to create socket" << std::endl;
        return;
    }

    ifreq ifr;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    // ifr.ifr_mtu = 15000;
    if (ioctl(sockfd, SIOCGIFMTU, &ifr) != 0)
    {
        perror("ioctl");
        return;
    }
    std::cout<< "MTU: " << ifr.ifr_mtu << std::endl;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;   // Use IPV4
    addr.sin_port = htons(port); //
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

void SocketBind(int &sockfd, struct sockaddr_in &addr, socklen_t &addr_len, int port)
{
    if (bind(sockfd, (struct sockaddr *)&addr, addr_len) == -1)
    {
        std::cout << "Failed to bind socket on port " << port << std::endl;
        close(sockfd);
        return;
    }
    else
    {
        // std::cout << "Bind socket on port " << port << std::endl;
    }
}

void SocketConnect(int &sockfd, struct sockaddr_in &addr, socklen_t &addr_len, int port)
{
    std::cout << "Connectting..." << port << std::endl;
    if (connect(sockfd, (struct sockaddr *)&addr, addr_len) == -1)
    {
        std::cout << "Failed to connect socket on port " << port << std::endl;
        close(sockfd);
        return;
    }
    else
    {
        std::cout << "Connect socket on port " << port << std::endl;
    }
}

void SocketListen(int &sockfd, int port)
{
    if (listen(sockfd, 5) < 0)
    {
        std::cout << "Failed to listen socket on port " << port << std::endl;
        close(sockfd);
        return;
    }
    else
    {
        std::cout << "Listen socket on port " << port << std::endl;
    }
}

void SocketAccept(int &sockfd, struct sockaddr_in &addr, socklen_t &addr_len, int port)
{
    struct sockaddr_in new_addr;
    socklen_t new_addr_len = sizeof(new_addr);
    std::cout << "Accepting..." << std::endl;
    int new_sockfd = accept(sockfd, (struct sockaddr *)&new_addr, &new_addr_len);
    if (new_sockfd < 0)
    {
        std::cout << "Failed to accept socket on port " << port << std::endl;
        close(sockfd);
        return;
    }
    else
    {
        std::cout << "Accept socket on port " << port << std::endl;
    }
    sockfd = new_sockfd;
}

void SocketRecv(int tcp_or_udp, int &sockfd)
{
    long long file_size = 1024 * 32;
    char *recv_buffer = new char[file_size];
    memset(recv_buffer, 0, strlen(recv_buffer));

    int counter = 0;
    struct sockaddr_in src;
    socklen_t src_len = sizeof(src);
    memset(&src, 0, sizeof(src));

    // 阻塞住接受消息
    std::fstream file_io_stream;
    file_io_stream.open("test.txt", std::ios::out | std::ios::binary);
    int sz = 0;
    int miss_counter = 0, packet_counter = 0;

    // 设置超时等待
    struct timeval tv;
    tv.tv_sec  = 2;
    tv.tv_usec = 0;  // 2 s
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
    if (tcp_or_udp == 0)
    {
        while (1)
        {
            // std::cout << "-----Read-----" << std::endl;
            sz = recvfrom(sockfd, recv_buffer, file_size, 0, (sockaddr *)&src, &src_len);
            if (sz > 1)
            {
                packet_counter++;
                sendto(sockfd, recv_buffer, sz, 0, (sockaddr *)&src, src_len);
                // file_io_stream << recv_buffer;
                // std::cout << "Recv File Stream Size: " << sz << " KBytes" << std::endl;
                // std::cout << "Recv File Times: " << packet_counter << std::endl;
                // std::cout << "Recv char: " << recv_buffer[0] << std::endl;
            }
            else
            {
                miss_counter++;
                std::cout << "MISS:  " << miss_counter << std::endl;
                if (miss_counter > 3)
                {
                    file_io_stream.close();
                    break;
                }
            }
        }
    }
    else
    {
        while (1)
        {
            // std::cout << "-----Read-----" << std::endl;
            sz = recv(sockfd, recv_buffer, file_size, 0);
            if (sz > 0)
            {
                packet_counter++;
                send(sockfd, recv_buffer, sz, 0);
                // file_io_stream << recv_buffer;
                // std::cout << "Recv File Stream Size: " << sz << " KBytes" << std::endl;
                // std::cout << "Recv File Times: " << packet_counter << std::endl;
                // std::cout << "Recv char: " << recv_buffer[0] << std::endl;
            }
            else if (sz == 0)
            {
                std::cout << "客户端停止发送数据..." << std::endl;
                break;
            }
            else
            {
                std::cout << "recv发生错误！" << std::endl;
            }
        }
    }
    // close(sockfd);
    delete[] recv_buffer;
    recv_buffer = nullptr;
}

void SocketSend(int tcp_or_udp, int &sockfd, struct sockaddr_in &addr, socklen_t &addr_len, int port, char *send_buffer)
{
    int packer_size = 1024 * 32;
    int packet_num = strlen(send_buffer) / packer_size + bool(strlen(send_buffer) % packer_size);
    // std::cout << "Send File Times:  " << packet_num << std::endl;
    int recv_back_num = 0;

    char *send_t = send_buffer;
    char *recv_buffer = new char[packer_size];
    int packet_num_t = packet_num;
    if (tcp_or_udp == 0)
    {
        while (packet_num_t--)
        {
            sendto(sockfd, send_t, packer_size, 0, (sockaddr *)&addr, addr_len);
            send_t += packer_size;
            int recv_back_len = recvfrom(sockfd, recv_buffer, packer_size, 0, (sockaddr *)&addr, &addr_len);
            if (recv_back_len < 1)
                continue;
            else
                recv_back_num++;
            // std::cout << "UDP Send File Stream Size:  " << strlen(send_buffer) << " KBytes" << std::endl;
            // std::cout << "UDP Send Recv Back Success:  " << 100 * static_cast<double>(recv_back_num) / packet_num << "%" << std::endl;
            // std::cout << "UDP Send Recv Back " << recv_back_num<< " " <<recv_buffer[0]<<" "<<recv_back_len <<  std::endl;
        }
    }
    else
    {
        while (packet_num_t--)
        {
            if (send(sockfd, send_t, packer_size, 0) < 0)
            {
                std::cout << "Send failed : " << sockfd << std::endl;
                return;
            }
            send_t += packer_size;
            int recv_back_len = recv(sockfd, recv_buffer, packer_size, 0);
            if (recv_back_len < 0)
                continue;
            else
                recv_back_num++;
            // std::cout << "TCP Send File Stream Size:  " << strlen(send_buffer) << " KBytes" << std::endl;
            // std::cout << "TCP Send Recv Back Success:  " << 100 * static_cast<double>(recv_back_num) / packet_num << "%" << std::endl;
            // std::cout << "TCP Send Recv Back " << recv_back_num << " " << recv_back_len << std::endl;
        }
    }
    // close(sockfd);
    delete[] recv_buffer;
    recv_buffer = nullptr;
}

void TimeCounter(void (*fp)(), int times = 1, TestType type = TestType::DickIO, int TestSize = MB_SIZE)
{
    std::cout << "\n=============TimeCounter=============" << std::endl;
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < times; i++)
    {
        fp();
    }

    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算耗时（毫秒）
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // 输出结果
    std::cout << "All Time: " << duration << " ms" << std::endl;
    std::cout << "Average Time: " << static_cast<double>(duration) / times << " ms" << std::endl;

    if (TestType::TCP == type || TestType::UDP == type)
        std::cout << "Average Speed: " << 2 * static_cast<double>(TestSize / MB_SIZE) * 1000 / (static_cast<double>(duration) / times) << " MB/s" << std::endl;
    else if (TestType::DickIO == type)
        std::cout << "Average Speed: " << static_cast<double>(GB_SIZE / MB_SIZE) * 1000 / (static_cast<double>(duration) / times) << " MB/s" << std::endl;

    std::cout << "=============TimeCounter=============\n"
              << std::endl;
}

#endif