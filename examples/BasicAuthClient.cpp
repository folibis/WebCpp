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
 * BasicAuthClient - a simple HTTP client application that sends GET requests
 * to the remote server that requires Basic authentication
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
#include "Lock.h"
#include "Signal.h"

#define USER_DEFAULT "foo"
#define PASSWORD_DEFAULT "bar"
#define REQUEST_COUNT 10

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
    WebCpp::Mutex mutex;
    WebCpp::Signal sig;

    signal(SIGINT, handle_sigint);

    httpCient.SetResponseCallback([&httpCient](const WebCpp::Response &response) -> bool
    {
        std::cout << "response code: " << response.GetResponseCode() << " " << response.GetResponsePhrase() << std::endl;

        StringUtil::PrintHex(response.GetBody());
        return true;
    });

    httpCient.SetProgressCallback([](size_t all, size_t downoaded)
    {
        std::cout << "downloaded :" << downoaded << " of " <<all << "\r";
    });

    httpCient.SetAuthCallback([&httpCient](const WebCpp::Request &request, WebCpp::AuthProvider &authProvider) -> bool
    {
        std::cout << "authentication required" << std::endl;
        for(auto const &scheme: authProvider.Get())
        {
            std::cout << "server auth method found: " << scheme->GetSchemeName() << std::endl;
            if(scheme->GetSchemeName() == "Basic")
            {
                std::cout << "Ok, server supports for Basic authentication, sending credentials" << std::endl;
                scheme->SetUser(USER_DEFAULT);
                scheme->SetPassword(PASSWORD_DEFAULT);
                return true;
            }
        }

        std::cout << "authentication isn't available" << std::endl;
        return false;
    });

    httpCient.SetStateCallback([&sig](const WebCpp::HttpClient::State state)
    {
        if(state == WebCpp::HttpClient::State::Closed ||
                state == WebCpp::HttpClient::State::Undefined)
        {
            sig.Fire();
        }
    });

    WebCpp::HttpConfig &config = WebCpp::HttpConfig::Instance();
    config.SetTempFile(true);
    config.SetMaxBodyFileSize(120_Mb);

    if(httpCient.Init())
    {
        httpCient.SetKeepOpen(false);
#ifdef WITH_OPENSSL
        std::string address("https://httpbin.org/basic-auth/");
#else
        std::string address("http://httpbin.org/basic-auth/");
#endif

        address +=  USER_DEFAULT;
        address += "/";
        address += PASSWORD_DEFAULT;

        StringUtil::RandInit();

        std::cout << "sending " << REQUEST_COUNT << " requests to " << address << std::endl;
        std::cout << "the authentication will be periodically randomly cleared" << std::endl;

        for(int i = 0;i < REQUEST_COUNT;i ++)
        {
            std::cout << (i + 1) << ". sending request to the server..." << std::endl;
            if(httpCient.Open(WebCpp::Http::Method::GET, address) == false)
            {
                std::cout << "connection failed: " << httpCient.GetLastError() << std::endl;
            }
            else
            {
                std::cout << "waiting for response..." << std::endl;
                WebCpp::Lock lock(mutex);
                sig.Wait(mutex);
                if(StringUtil::GetRand(0, 5) == 0)
                {
                    httpCient.ClearAuth();
                    std::cout << "clearing the previous authentication" << std::endl;
                }
            }
        }
        httpCient.Close(true);
    }
    else
    {
        WebCpp::DebugPrint() << "HTTP client Init() failed: " << httpCient.GetLastError() << std::endl;
    }

    std::cout << "exiting" << std::endl;
}
