#include <iostream>
#include <event2/event.h>
#include <signal.h>

void timer1(evutil_socket_t, short, void *)
{
    std::cout << "timer1" << std::endl;
}

void timer2(evutil_socket_t, short, void *)
{
    std::cout << "timer2" << std::endl;
}

void timer3(evutil_socket_t, short, void *)
{
    std::cout << "timer3" << std::endl;
}

int test_timer(event_base *base)
{

    // 非持久处理器
    event *ev1 = evtimer_new(base, timer1, nullptr);
    timeval t1 = {1, 0};
    evtimer_add(ev1, &t1);

    // 持久定时器 默认使用二叉堆(O(log n))
    event *ev2 = event_new(base, -1, EV_PERSIST, timer2, nullptr);
    timeval t2 = {3, 0};
    evtimer_add(ev2, &t2);

    // 改变定时器结构
    // event_base_init_common_timeout是Libevent提供的一个性能优化函数，主要解决大量相同超时时间定时器
    event *ev3 = event_new(base, -1, EV_PERSIST, timer3, nullptr);
    timeval tv_in = {6, 0};
    const timeval *t3 = event_base_init_common_timeout(base, &tv_in);
    // 优化后使用队列/时间轮(O(1))
    evtimer_add(ev3, t3);

    event_base_dispatch(base);

    event_free(ev1);
    event_free(ev2);
    event_free(ev3);

    return 0;
}