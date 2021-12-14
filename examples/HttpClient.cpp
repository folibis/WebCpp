#include <signal.h>
#include "common_webcpp.h"
#include "HttpClient.h"
#include "Request.h"
#include <string>
#include <iostream>
#include "StringUtil.h"
#include "FileSystem.h"
#include "Url.h"
#include "fstream"
#include "Data.h"
#include "DebugPrint.h"


static WebCpp::HttpClient *ptr = nullptr;

void handle_sigint(int)
{
    if(ptr != nullptr)
    {
        ptr->Close(false);
    }
}

int main()
{
    WebCpp::HttpClient httpCient;
    ptr = &httpCient;

    signal(SIGINT, handle_sigint);

    httpCient.SetResponseCallback([&httpCient](const WebCpp::Response &response) -> bool
    {
        std::cout << "response code: " << response.GetResponseCode() << " " << response.GetResponsePhrase() << std::endl;

        StringUtil::Print(response.GetBody());
        httpCient.Close(false);
        return true;
    });

    httpCient.SetProgressCallback([](size_t all, size_t downoaded) {
        std::cout << "downloaded :" << downoaded << " of " <<all << "\r";
    });

    WebCpp::HttpConfig config;
    config.SetTempFile(true);
    config.SetMaxBodyFileSize(120_Mb);

    if(httpCient.Init(config))
    {
#ifdef WITH_OPENSSL
        std::string address("https://www.google.com");
#else
        std::string address("http://www.google.com");
#endif
        httpCient.Open(WebCpp::Http::Method::GET, address);
    }
    else
    {
        WebCpp::DebugPrint() << "HTTP client Init() failed: " << httpCient.GetLastError() << std::endl;
    }

    httpCient.WaitFor();

}
