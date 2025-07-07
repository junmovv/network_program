#include "head.h"

void print_features(event_base *base)
{
    // event_base_get_features  获取当前 event_base 实例所支持的事件处理特性 注意这个只是支持 并不一定启用了
    int features = event_base_get_features(base);
    std::cout << "support features:" << std::endl;
    std::cout << "  EV_FEATURE_ET: " << ((features & EV_FEATURE_ET) ? "YES" : "NO") << std::endl;
    std::cout << "  EV_FEATURE_O1: " << ((features & EV_FEATURE_O1) ? "YES" : "NO") << std::endl;
    std::cout << "  EV_FEATURE_FDS: " << ((features & EV_FEATURE_FDS) ? "YES" : "NO") << std::endl;

    std::cout << "enable I/O: " << event_base_get_method(base) << std::endl;
}

event_base *config_new(int useCfg = false, int fds = false)
{
    event_base *base = nullptr;

    if (!useCfg)
    {
        base = event_base_new();
        if (!base)
        {
            std::cerr << "base new fail" << std::endl;
            return nullptr;
        }
        std::cout << "使用默认配置创建 event_base" << std::endl;
        print_features(base);
        return base;
    }

    std::cout << "自定义配置创建 event_base" << std::endl;

    // 创建配置对象
    event_config *cfg = event_config_new();
    if (!cfg)
    {
        std::cerr << "event_config_new failed!" << std::endl;
        return nullptr;
    }

    // 调试： 显示系统支持的后端I/O方法
    const char **methods = event_get_supported_methods();
    std::cout << "Supported I/O methods:" << std::endl;
    for (int i = 0; methods[i] != NULL; i++)
    {
        std::cout << "  " << methods[i] << std::endl;
    }

    // 设置特性  支持任意文件描述符（非仅socket）
    // 设置了EV_FEATURE_FDS 其他特征就无法设置，例如旧内核的 epoll 不支持非socket fd 的 ET 模式
    if (fds && event_config_require_features(cfg, EV_FEATURE_FDS) != 0)
    {
        std::cerr << "EV_FEATURE_FDS not supported " << std::endl;
        event_config_free(cfg);
        return nullptr;
    }

    // event_config_require_features(cfg,  EV_FEATURE_ET); // 设置ET

    /// 设置避免使用的网络模型  epoll > kqueue > poll > select
    // event_config_avoid_method(cfg, "poll");
    // event_config_avoid_method(cfg, "epoll");

    // 应用配置创建事件基础 Linux默认是 epoll LT模式 前提未启用 EV_FEATURE_FDS
    base = event_base_new_with_config(cfg);
    event_config_free(cfg);

    if (!base)
    {

        // 自定义配置失败时回退到默认配置
        std::cerr << "event_base_new_with_config fail" << std::endl;
        base = event_base_new();
        if (!base)
        {
            std::cerr << "base new fail" << std::endl;
            return nullptr;
        }
    }
    print_features(base);
    return base;
}