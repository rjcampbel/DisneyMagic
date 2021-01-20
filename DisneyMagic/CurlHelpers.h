#include <curl/curl.h>
#include <string>

namespace curlhelpers
{
    int retrieve_file_from_URL(const std::string& url, std::string& fileBuffer);
}