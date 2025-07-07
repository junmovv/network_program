#include "config.hpp"
#include "signal.hpp"
#include "timer.hpp"
#include "file.hpp"
#include "server.hpp"
int main()
{
    // 忽略SIGPIPE信号（防止写关闭的socket导致进程退出）
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        std::cerr << "Failed to ignore SIGPIPE" << std::endl;
        return -1;
    }
    event_base *base = config_new();
    if (!base)
    {
        return -1;
    }
    test_signal(base);
    test_timer(base);
    test_file(base);
    test_server(base);
    event_base_free(base);

    return 0;
}