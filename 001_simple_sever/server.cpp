#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/prctl.h>
#include "server.h"
#include <string>
#include <iostream>
#include <thread>

#define SAFE_CLOSE(fd) \
    do                 \
    {                  \
        if (-1 != fd)  \
        {              \
            close(fd); \
            (fd) = -1; \
        }              \
    } while (0)

#define SERVER_PORT 12345 // 端口号
#define BACKLOG 24        // 最大连接数
#define SUCCESS 0
#define FAILURE -1

void setPthreadName(char *name)
{
    if (name != NULL)
    {
        prctl(PR_SET_NAME, (unsigned long)name); // name 最多15个字符
    }
}

void getPthreadName(char *name)
{
    if (name)
    {
        prctl(PR_GET_NAME, (unsigned long)name); // max 15 字符
    }
}

int Server::tcpServerInit(int &sockfd)
{
    int ret = FAILURE;
    struct sockaddr_in serverAddr;
    int addreuse = 1;

    do
    {
        // 创建 socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            printf("socket open failed\n");
            break;
        }

        // 设置 SO_REUSEADDR 选项
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &addreuse, sizeof(int)) == FAILURE)
        {
            printf("setsockopt failed\n");
            break;
        }

        bzero((char *)&serverAddr, sizeof(struct sockaddr_in));

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SERVER_PORT);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in)) != 0)
        {
            printf("bind failed\n");
            break;
        }

        if (listen(sockfd, BACKLOG) != 0)
        {
            printf("listen failed\n");
            break;
        }

        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) != 0)
        {
            printf("setsockopt timeout failed\n");
            break;
        }

        ret = SUCCESS;
    } while (0);

    if (ret == FAILURE && sockfd >= 0)
    {
        close(sockfd);
        sockfd = -1;
    }

    return ret;
}

std::string Server::readHttpHeader(int sockfd)
{
    std::string headerData;
    char buffer;
    fd_set readfds;
    struct timeval timeout;
    int ret;

    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        timeout.tv_sec = 5; // 设置超时时间为 5 秒
        timeout.tv_usec = 0;

        ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (ret == -1)
        {
            std::cerr << "Error in select" << std::endl;
            break;
        }
        else if (ret == 0)
        {
            std::cerr << "Timeout reached" << std::endl;
            break;
        }
        else
        {
            if (read(sockfd, &buffer, 1) > 0)
            {
                headerData += buffer;
                // 检查是否已经读取到头部结束标志
                if (headerData.size() >= 4 && headerData.substr(headerData.size() - 4) == "\r\n\r\n")
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    return headerData;
}

void Server::processRequest(void *p, int connfd)
{
    if (NULL == p)
    {
        return;
    }
    Server *pServer = static_cast<Server *>(p);

    char pname[16] = {0};
    snprintf(pname, sizeof(pname), "%s_%d", "process", connfd);
    setPthreadName(pname);

    struct timeval select_timeout;
    select_timeout.tv_sec = 2;
    select_timeout.tv_usec = 0;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(connfd, &rset);
    if (select(connfd + 1, &rset, NULL, NULL, &select_timeout) > 0)
    {
        if (!FD_ISSET(connfd, &rset))
        {
            goto errExit;
        }

        std::string head = pServer->readHttpHeader(connfd);
        std::cout << head << std::endl;
    }

errExit:

    SAFE_CLOSE(connfd);

    return;
}

void Server::thread_ServerListenFrom(void *p)
{
    if (NULL == p)
    {
        return;
    }

    Server *pServer = static_cast<Server *>(p);
    char pname[16] = {0};
    sprintf(pname, "ServerListen");
    setPthreadName(pname);

    struct timeval timeout;
    int sockfd = -1, connfd = -1;
    pServer->tcpServerInit(sockfd);

    while (1)
    {

        if ((connfd = accept(sockfd, NULL, NULL)) < 0)
        {
            usleep(500000);
            printf("Accept() failed errno=%d.\n", errno);
            continue;
        }

        std::thread t(&Server::processRequest, pServer, connfd);
        t.detach();
    }

exit:
    SAFE_CLOSE(sockfd);
}

int main()
{
    Server *pSever = new Server;
    std::thread t1(&Server::thread_ServerListenFrom, pSever);
    t1.join();
    delete pSever;
    pSever = nullptr
    return 0;
}