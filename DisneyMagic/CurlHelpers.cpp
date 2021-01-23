#include "CurlHelpers.h"

namespace curlhelpers
{

size_t write_data(char *data, size_t memberSize, size_t memberCount, std::string *destination)
{
    size_t size = memberSize * memberCount;
    destination->append(data, size);
    return size;
}

void retrieve_file_from_URL(const std::string& url, std::string& fileBuffer)
{
    CURL *curl = curl_easy_init();
    if (curl != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileBuffer);
        CURLcode result = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (result != CURLE_OK)
        {
            return std::runtime_error("Curl perform failed with code: " + std::to_string(result));
        }
    }
    else
    {
        throw std::runtime_error("Curl init failed");
    }
}

}
