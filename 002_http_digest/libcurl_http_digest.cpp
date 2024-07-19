#include <curl/curl.h>
#include <string>
#include <cstdio>
#include <jsoncpp/json/json.h>

class licurl_http_digest
{
private:
    std::string responseContent;
    std::string responseHeader;

    std::string url_;
    std::string user_;
    std::string password_;

public:
    licurl_http_digest(const std::string &url, const std::string &user, const std::string &pwd);
    ~licurl_http_digest() = default;
    static size_t write_data(void *ptr, size_t size, size_t nmemb, std::string *data);
    std::string pack_http_json();
};

size_t licurl_http_digest::write_data(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    std::string &str = *(std::string *)data;
    str.append((char *)ptr, size * nmemb);
    return size * nmemb;
}

licurl_http_digest::licurl_http_digest(const std::string &url, const std::string &user, const std::string &pwd)
    : url_(url), user_(user), password_(pwd)
{

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        printf("url %s\n", url_.c_str());
        // 设置Digest认证
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);

        // 设置请求连接超市时间
        res = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
        // 设置连接超时时间
        res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

        // 设置用户名和密码
        curl_easy_setopt(curl, CURLOPT_USERNAME, user_.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password_.c_str());

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
}

std::string licurl_http_digest::pack_http_json()
{
    Json::Value jsonData;
    jsonData["name"] = "John Doe";
    jsonData["age"] = 30;

    return jsonData.toStyledString();
}

int main()
{
    licurl_http_digest("https://example.com/api/resource", "root", "123");
    return 0;
}