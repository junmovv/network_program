#include <curl/curl.h>
#include <string>
#include <cstdio> // Assuming this is for printf/INFO macros

// Global variables for response data
std::string responseContent;
std::string responseHeader;

std::string ip = "192.168.1.2";
std::string user = "root";
std::string password = "abc123";
std::string urlPath = "hello";

size_t write_data(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    // Your implementation of write_data function
    return size * nmemb; // Example implementation
}

std::string pack_http_json()
{
    return "hello world"; // Example implementation
}
int main()
{

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        std::string url = std::string("http://") + ip ;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        printf("url %s\n", url.c_str());
        // 设置Digest认证
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);

        // 设置请求连接超市时间
        res = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
        // 设置连接超时时间
        res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

        // 设置用户名和密码
        curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());

        // 设置请求方法为PUT
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

        // 设置PUT请求的数据体
        std::string data = pack_http_json();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        // 设置接收报文数据的回调
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        // 设置数据接收的实体
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseContent);

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data);

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeader);

        // 发送请求
        res = curl_easy_perform(curl);

        // 检查结果
        if (res != CURLE_OK)
        {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        printf("responseContent \n %s\n", responseContent.c_str());
        // 清理curl句柄
        curl_easy_cleanup(curl);
        }
    return 0;
}