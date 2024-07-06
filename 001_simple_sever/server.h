#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server {
public:
    Server() = default;
    ~Server() = default;
    static void thread_ServerListenFrom(void *p);
    
private:
    int tcpServerInit(int &sockfd);
    std::string readHttpHeader(int sockfd);
    static void processRequest(void *p, int connfd);
};

#endif // SERVER_H