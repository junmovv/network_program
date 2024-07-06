#include <iostream>
#include <openssl/md5.h>
#include <iomanip>
#include <sstream>
#include <cstring>

#include <string>
#include <unordered_map>

struct DigestAuthentication
{
    std::string realm;
    std::string nonce;
    std::string opaque;
    std::string algorithm;
    std::string qop;
};

// Function to parse WWW-Authenticate header and extract digest authentication fields
DigestAuthentication parseDigestAuthentication(const std::string &wwwAuthenticateHeader)
{
    DigestAuthentication auth;

    // Find "Digest" keyword
    size_t pos = wwwAuthenticateHeader.find("Digest");
    if (pos != std::string::npos)
    {
        // Extract substring after "Digest"
        std::string digestInfo = wwwAuthenticateHeader.substr(pos + strlen("digest")); // Length of "Digest "

        // Initialize stringstream for parsing
        std::stringstream ss(digestInfo);

        // Map to store key-value pairs
        std::unordered_map<std::string, std::string *> keyValueMap;

        keyValueMap["realm"] = &auth.realm;
        keyValueMap["nonce"] = &auth.nonce;
        keyValueMap["opaque"] = &auth.opaque;
        keyValueMap["algorithm"] = &auth.algorithm;
        keyValueMap["qop"] = &auth.qop;

        // Read pairs like key="value"
        std::string pair;
        while (std::getline(ss, pair, ','))
        {
            // Split each pair into key and value
            size_t eqPos = pair.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key = pair.substr(1, eqPos - 1); // 1为了跳过空格
                std::string value;

                // Extract value, handling quotes
                size_t startValue = pair.find_first_of('"', eqPos);
                size_t endValue = pair.find_last_of('"');
                if (startValue != std::string::npos && endValue != std::string::npos)
                {
                    value = pair.substr(startValue + 1, endValue - startValue - 1);
                }

                if (keyValueMap.find(key) != keyValueMap.end())
                {
                    *keyValueMap[key] = value;
                }
            }
        }
        for (const auto &elem : keyValueMap)
        {
            std::cout << elem.first << "**********" << *elem.second << std::endl;
        }
    }

    return auth;
}

void MD5toStr(unsigned char *src, int len, char *out)
{
    int i;
    int msb = 0, lsb = 0;
    char temp[2 * len + 1];
    const char tab[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    if ((NULL == src) || (NULL == out))
    {
        return;
    }

    for (i = 0; i < len; i++)
    {
        lsb = (*(src + i)) & 0xf;
        msb = ((*(src + i)) & 0xf0) >> 4;
        temp[2 * i] = tab[msb];
        temp[2 * i + 1] = tab[lsb];
    }
    temp[2 * i] = '\0';
    memcpy(out, temp, 2 * len + 1);

    return;
}

std::string md5(const std::string &input)
{
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, input.c_str(), input.length());

    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &context);

    char digetStr[64] = {0};

    MD5toStr(digest, MD5_DIGEST_LENGTH, digetStr);

    std::cout << "digestStr " << digetStr << std::endl;

    std::stringstream ss;
    //  十六进制输出模式  每个输出的宽度为2个字符  设置填充字符为 ‘0’
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }

    return ss.str();
}

int main()
{
    // Example WWW-Authenticate header
    std::string wwwAuthenticateHeader = R"##(Digest qop="auth", realm="IP Camera(FA451)", nonce="613066623a38333263356136373a150a14a81c3b8e3a265dabbba22e3ec1", stale="FALSE")##";

    // Parse the WWW-Authenticate header
    DigestAuthentication auth = parseDigestAuthentication(wwwAuthenticateHeader);

    // Output parsed fields
    std::cout << "Realm: " << auth.realm << std::endl;
    std::cout << "Nonce: " << auth.nonce << std::endl;
    std::cout << "Opaque: " << auth.opaque << std::endl;
    std::cout << "Algorithm: " << auth.algorithm << std::endl;
    std::cout << "QOP: " << auth.qop << std::endl;

    std::string input = "Hello, MD5!";
    std::string result = md5(input);

    std::cout << "MD5 hash of '" << input << "' is: " << result << std::endl;

    return 0;
}
