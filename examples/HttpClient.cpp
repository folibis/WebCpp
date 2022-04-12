/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

/*
 * HttpClient - a simple HTTP client application that sends GET request
 * to the remote server
*/

#include <csignal>
#include "common_webcpp.h"
#include "HttpClient.h"
#include "Request.h"
#include "StringUtil.h"
#include "FileSystem.h"
#include "Url.h"
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

        StringUtil::PrintHex(response.GetBody());
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
