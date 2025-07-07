#include "head.h"
#define use_socket

void close_client(event *ev, evutil_socket_t fd)
{
    if (ev)
        event_free(ev);
    if (fd >= 0)
        evutil_closesocket(fd);
}

void client_cb(evutil_socket_t fd, short events, void *arg)
{
    event *ev = (event *)arg;

    if (events & EV_TIMEOUT)
    {
        std::cout << "Connection timeout" << std::endl;
        close_client(ev, fd);
        return;
    }

    char buf[1024] = {0};
    int len = ::recv(fd, buf, sizeof(buf) - 1, 0);

    if (len > 0)
    {
        std::cout << "Received: " << buf << std::endl;
        if (::send(fd, "ok", 2, 0) < 0 && errno != EAGAIN)
        {
            std::cerr << "send error: " << strerror(errno) << std::endl;
        }
    }
    else if (len == 0)
    {
        std::cout << "Client disconnected" << std::endl;
        close_client(ev, fd);
    }
    else
    {
        if (errno != EAGAIN)
        {
            std::cerr << "recv error: " << strerror(errno) << std::endl;
            close_client(ev, fd);
        }
    }
}

#ifdef use_socket
void accept_cb(evutil_socket_t listen_fd, short events, void *arg)
#else
void accept_cb(evconnlistener *listener, evutil_socket_t clientFd, sockaddr *sa, int socklen, void *arg)
#endif
{
#ifdef use_socket
    sockaddr_in sinaddr;
    socklen_t size = sizeof(sinaddr);
    int clientFd = ::accept(listen_fd, (sockaddr *)&sinaddr, &size);

    if (clientFd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "accept error: " << strerror(errno) << std::endl;
        }
        return;
    }

    evutil_make_socket_nonblocking(clientFd);
    sockaddr_in *sin = &sinaddr;
#else
    sockaddr_in *sin = (sockaddr_in *)sa;
#endif
    char ip[32] = {0};
    evutil_inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip) - 1);
    std::cout << "Client connected from: " << ip << std::endl;

    event_base *base = (event_base *)arg;
    event *ev = event_new(base, clientFd, EV_READ | EV_PERSIST, client_cb, event_self_cbarg());

    if (!ev)
    {
        std::cerr << "Failed to create client event" << std::endl;
        evutil_closesocket(clientFd);
        return;
    }

    timeval timeout = {10, 0};
    if (event_add(ev, &timeout) != 0)
    {
        std::cerr << "Failed to add client event" << std::endl;
        close_client(ev, clientFd);
    }
}

void error_cb(evconnlistener *listener, void *arg)
{
    event_base *base = (event_base *)arg;
    std::cerr << "Listener error: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << std::endl;
    timeval timeout = {3, 0};
    event_base_loopexit(base, &timeout);
}

int test_server(event_base *base)
{

#ifdef use_socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd <= 0)
    {
        std::cerr << "socket error: " << strerror(errno) << std::endl;
        return -1;
    }

    evutil_make_socket_nonblocking(listen_fd);
    evutil_make_listen_socket_reuseable(listen_fd);

    sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8000);
    if (::bind(listen_fd, (sockaddr *)&sin, sizeof(sin)) != 0)
    {
        std::cerr << "bind error: " << strerror(errno) << std::endl;
        evutil_closesocket(listen_fd);
        return -1;
    }

    if (::listen(listen_fd, 128) != 0)
    {
        std::cerr << "listen error: " << strerror(errno) << std::endl;
        evutil_closesocket(listen_fd);
        return -1;
    }

    event *ev = event_new(base, listen_fd, EV_READ | EV_PERSIST, accept_cb, base);
    if (!ev || event_add(ev, NULL) != 0)
    {
        std::cerr << "Failed to create listener event" << std::endl;
        if (ev)
            event_free(ev);
        evutil_closesocket(listen_fd);
        return -1;
    }
#else
    sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8000);
    // 封装了 socket bind listen event_new event_add
    evconnlistener *listener = evconnlistener_new_bind(
        base, accept_cb, base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        -1, (sockaddr *)&sin, sizeof(sin));

    if (!listener)
    {
        std::cerr << "Failed to create listener: "
                  << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR())
                  << std::endl;
        event_base_free(base);
        return -1;
    }
    evconnlistener_set_error_cb(listener, error_cb);
#endif

    std::cout << "Server started on port 8000..." << std::endl;
    event_base_dispatch(base);

#ifdef use_socket
    event_free(ev);
    evutil_closesocket(listen_fd);
#else
    evconnlistener_free(listener);
#endif

    return 0;
}