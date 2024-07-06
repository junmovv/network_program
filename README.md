# network_program

### 000_codedump生成 和 openssl 安装

~~~shell
#检查当前shell会话的核心转储文件大小限制：
ulimit -c
#取消核心转储文件大小的限制（设置为无限制）
ulimit -c unlimited

#查看当前的core_pattern设置
cat /proc/sys/kernel/core_pattern

#修改core_pattern（需要root权限），例如设置为在当前目录下生成名为core的文件
sudo sh -c 'echo "core" > /proc/sys/kernel/core_pattern'

#使用 apt 安装 OpenSSL
sudo apt update
sudo apt install openssl
~~~

