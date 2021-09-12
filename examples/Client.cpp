#include <signal.h>
#include "common.h"
#include "HttpClient.h"
#include "Request.h"
#include <string>
#include <iostream>
#include "StringUtil.h"
#include "FileSystem.h"
#include "Url.h"
#include "fstream"
#include "Data.h"


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

    WebCpp::HttpConfig config;
    config.SetHttpProtocol("HTTP");
    config.SetHttpServerPort(8080);

    httpCient.SetResponseCallback([](const WebCpp::Response &response) -> bool
    {
        std::cout << "response code: " << response.GetResponseCode() << " " << response.GetResponsePhrase() << std::endl;

        auto str = StringUtil::ByteArray2String(response.GetBody());
        std::cout << str << std::endl;
        return true;
    });

    httpCient.Run();

    if(httpCient.Init(config))
    {
        httpCient.Open(WebCpp::Http::Method::GET, "http://www.google.com");
    }

    httpCient.WaitFor();

}
