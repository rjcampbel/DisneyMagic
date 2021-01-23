#pragma once

#include <curl/curl.h>
#include <string>
#include <exception>

namespace curlhelpers
{
    void retrieve_file_from_URL(const std::string& url, std::string& fileBuffer);
}