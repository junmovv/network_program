#include "head.h"

void ctrl_c(evutil_socket_t fd, short events, void *arg)
{
    static int cnum = 0;
    std::cout << "\nSIGINT(Ctrl+C) received. Exiting..." << std::endl;
    event_base *base = static_cast<event_base *>(arg);
    if (++cnum >= 3)
    {
        timeval timeout = {3, 0};
        event_base_loopexit(base, &timeout);
    }
}

void on_kill(evutil_socket_t fd, short events, void *arg)
{

    static int cnum = 0;
    std::cout << "\nSIGTERM(kill) received. Exiting..." << std::endl;
    event *ev = static_cast<event *>(arg);
    if (!event_pending(ev, EV_SIGNAL, nullptr))
    {
        event_add(ev, nullptr);
    }
    if (++cnum >= 3)
    {
        event_base *base = event_get_base(ev);
        timeval timeout = {3, 0};
        event_base_loopexit(base, &timeout);
    }
}

int test_signal(event_base *base)
{

    // 2. 创建SIGINT事件 (Ctrl+C)
    event *sigint_event = event_new(base, SIGINT, EV_SIGNAL | EV_PERSIST, ctrl_c, base);
    // 传递base作为回调参数
    if (!sigint_event || event_add(sigint_event, nullptr) < 0)
    {
        std::cerr << "Failed to create/add SIGINT event" << std::endl;
        return 1;
    }

    // 3. 创建SIGTERM事件 (kill命令)
    event *sigterm_event = event_new(base, SIGTERM, EV_SIGNAL, on_kill, event_self_cbarg());

    if (!sigterm_event || event_add(sigterm_event, nullptr) < 0)
    {
        std::cerr << "Failed to create/add SIGTERM event" << std::endl;
        event_free(sigint_event);
        return 1;
    }

    std::cout << "Signal handler running. Press Ctrl+C or use 'kill' to terminate." << std::endl;

    event_base_dispatch(base);

    event_free(sigint_event);
    event_free(sigterm_event);
}
