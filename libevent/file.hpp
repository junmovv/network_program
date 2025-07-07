#include "head.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <sys/inotify.h> // 添加inotify支持

std::atomic<bool> g_running(true);

struct FileState
{
    evutil_socket_t fd;
    off_t last_pos;
    event *file_event; // 关联的文件事件
    int inotify_fd;    // inotify描述符
};

void signal_handler(evutil_socket_t, short, void *arg)
{
    event_base *base = static_cast<event_base *>(arg);
    std::cout << "\nShutting down..." << std::endl;
    g_running = false;
    event_base_loopbreak(base);
}

// 文件修改回调（当文件被写入时触发）
void inotify_callback(evutil_socket_t inotify_fd, short, void *arg)
{
    FileState *state = static_cast<FileState *>(arg);

    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;

    // 读取inotify事件
    ssize_t len = read(inotify_fd, buf, sizeof(buf));
    if (len <= 0)
        return;

    // 处理所有事件
    for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len)
    {
        event = reinterpret_cast<const struct inotify_event *>(ptr);

        if (event->mask & IN_MODIFY)
        {
            // 文件被修改，重新激活文件读取事件
            event_add(state->file_event, nullptr);
            std::cout << "File modified, reactivating read handler\n";
        }
    }
}

void read_file(evutil_socket_t fd, short, void *arg)
{
    FileState *state = static_cast<FileState *>(arg);
    struct stat file_stat;

    if (fstat(fd, &file_stat) < 0)
    {
        perror("fstat failed");
        return;
    }

    // 检查是否有新数据
    if (file_stat.st_size > state->last_pos)
    {
        lseek(fd, state->last_pos, SEEK_SET);
        char buf[1024];

        while (true)
        {
            ssize_t nread = read(fd, buf, sizeof(buf) - 1);
            if (nread <= 0)
                break;

            buf[nread] = '\0';
            std::cout << buf;
            state->last_pos += nread;
        }
    }

    // 到达当前文件末尾（但保持监听）
    if (state->last_pos == file_stat.st_size)
    {
        // 仅注销事件但不释放资源
        event_del(state->file_event);
        std::cout << "Reached EOF, waiting for new writes...\n";
    }
}

int test_file(event_base *base)
{
    const char *filename = "./log";

    // 1. 打开日志文件
    int fd = open(filename, O_RDWR | O_CREAT | O_NONBLOCK, 0644);
    if (fd < 0)
    {
        perror("open failed");
        return -1;
    }

    // 2. 定位到文件末尾
    off_t initial_pos = lseek(fd, 0, SEEK_END);

    // 3. 初始化状态结构体
    FileState *state = new FileState();
    state->fd = fd;
    state->last_pos = initial_pos;

    // 4. 创建文件读取事件（非持久）
    state->file_event = event_new(base, fd, EV_READ, read_file, state);
    event_add(state->file_event, nullptr);

    // 5. 设置inotify监控文件修改
    state->inotify_fd = inotify_init1(IN_NONBLOCK);
    if (state->inotify_fd < 0)
    {
        perror("inotify_init failed");
        close(fd);
        delete state;
        return -2;
    }

    // 添加对文件修改的监控
    int wd = inotify_add_watch(state->inotify_fd, filename, IN_MODIFY);
    if (wd < 0)
    {
        perror("inotify_add_watch failed");
        close(state->inotify_fd);
        close(fd);
        delete state;
        return -3;
    }

    // 6. 创建inotify事件
    event *inotify_event = event_new(
        base,
        state->inotify_fd,
        EV_READ | EV_PERSIST,
        inotify_callback,
        state);
    event_add(inotify_event, nullptr);

    // 7. 设置信号处理
    event *sigint = evsignal_new(base, SIGINT, signal_handler, base);
    event *sigterm = evsignal_new(base, SIGTERM, signal_handler, base);
    event_add(sigint, nullptr);
    event_add(sigterm, nullptr);

    std::cout << "Monitoring " << filename << " for continuous writes\n"
              << "Press Ctrl+C to exit" << std::endl;

    // 8. 启动事件循环
    event_base_dispatch(base);

    // 9. 清理资源
    inotify_rm_watch(state->inotify_fd, wd);
    close(state->inotify_fd);
    event_free(inotify_event);
    event_free(sigterm);
    event_free(sigint);
    event_free(state->file_event);
    close(fd);
    delete state;

    std::cout << "Clean exit" << std::endl;
    return 0;
}