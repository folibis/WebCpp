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
 * LoadTest - a multithreaded HTTP client that simultaneously connects to a remote server
 * and calculates the average execution time.
*/

#include <csignal>
#include <string>
#include <unistd.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "common_webcpp.h"
#include "HttpClient.h"
#include "Request.h"
#include "StringUtil.h"
#include "FileSystem.h"
#include "Url.h"
#include "Data.h"
#include "DebugPrint.h"
#include "ThreadWorker.h"
#include "example_common.h"

#define DEFAULT_CLIENT_COUNT 10
#define DEFAULT_RESOURCE "http://httpbin.org/get"
#define DEFAULT_DELAY 200
#define TEST_COUNT 100
#define PRINT_RESPONSE (false)


size_t clientCount = DEFAULT_CLIENT_COUNT;
std::string resource = DEFAULT_RESOURCE;
long delay = DEFAULT_DELAY;
int testCount = TEST_COUNT;
bool printResponse = PRINT_RESPONSE;
bool g_running = true;
static int g_id = 0;
struct Result
{
    int requests = 0;
    long total_duration = 0;
    long long total_bytes = 0;
};
static std::vector<Result> results;

void handle_sigint(int)
{
    g_running = false;
}

void *ThreadRoutine(bool &running)
{
    int id = g_id++;
    int cnt = 0;
    std::cout << "starting thread " << id << std::endl;

    try
    {
        WebCpp::HttpClient httpCient;
        WebCpp::HttpConfig &config = WebCpp::HttpConfig::Instance();
        config.SetTempFile(true);
        config.SetMaxBodyFileSize(10_Mb);
        std::chrono::steady_clock::time_point start;
        std::chrono::steady_clock::time_point end;

        WebCpp::DebugPrint::AllowPrint = true;

        if(httpCient.Init())
        {
            httpCient.SetResponseCallback([&httpCient,id,&start,&end](const WebCpp::Response &response) -> bool
            {
                end = std::chrono::steady_clock::now();
                long dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                long size = response.GetHeader().GetRequestSize();
                results[id].requests ++;
                results[id].total_duration += dur;
                results[id].total_bytes += size;
                std::stringstream stream;
                stream
                        << id
                        << ": response code: "
                        << response.GetResponseCode() << (response.GetResponseCode() == 302 ? (" (" + response.GetHeader().GetHeader(WebCpp::HttpHeader::HeaderType::Location) + ")") :  "")
                        << ", size:  " << size
                        << ", duration: " << dur << " µs."
                        << std::endl;
                std::cout << stream.str();

                if(printResponse == true)
                {
                    StringUtil::PrintHex(response.GetBody());
                }

                return true;
            });

            while(running && g_running)
            {
                start = std::chrono::steady_clock::now();
                if(httpCient.Open(WebCpp::Http::Method::GET, resource) == true)
                {
                    usleep(delay * 1000);
                    cnt ++;
                    if(cnt >= testCount)
                    {
                        running = false;
                    }
                }
                else
                {

                    std::cout << "thread " << id << " failed to send request: " << httpCient.GetLastError() << std::endl;
                    running = false;
                }
            }

            httpCient.Close(false);
        }
    }
    catch(...)
    {
        running = false;
    }

    std::stringstream stream;
    stream << "finishing thread " << id << std::endl;
    std::cout << stream.str();

    return nullptr;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);

    auto cmdline = CommandLine::Parse(argc, argv);

    if(cmdline.Exists("-h"))
    {
        std::vector<std::string> adds;
        adds.push_back("-c: count of clients, default: " + std::to_string(DEFAULT_CLIENT_COUNT));
        adds.push_back("-r: URL of the resource to test, default: " + std::string(DEFAULT_RESOURCE));
        adds.push_back("-d: delay between requests, ms, default: " + std::to_string(DEFAULT_DELAY));
        adds.push_back("-n: count of tests, default: " + std::to_string(TEST_COUNT));
        adds.push_back("-p: print out the response, default: " + std::to_string(PRINT_RESPONSE));

        cmdline.PrintUsage(false, false, adds);
        exit(0);
    }

    int v;
    if(StringUtil::String2int(cmdline.Get("-c"), v))
    {
        clientCount = v;
    }

    if(cmdline.Exists("-r"))
    {
        resource = cmdline.Get("-r");
    }

    if(StringUtil::String2int(cmdline.Get("-d"), v))
    {
        delay = v;
    }

    if(StringUtil::String2int(cmdline.Get("-n"), v))
    {
        testCount = v;
    }
    if(cmdline.Exists("-p"))
    {
        printResponse = true;
    }

    std::cout << "testung URL: " << resource << std::endl;
    std::vector<WebCpp::ThreadWorker> workers;
    workers.resize(clientCount);
    results.resize(clientCount);
    for(size_t i = 0;i < clientCount;i ++)
    {
        auto &worker = workers.at(i);
        worker.SetFunction(ThreadRoutine);
        worker.Start();
    }

    for(size_t i = 0;i < clientCount;i ++)
    {
        auto &worker = workers.at(i);
        worker.Wait();
    }

    std::stringstream stream;

    stream << std::endl << "Results:" << "\n";
    stream << "|  id  | total, req. | total, bytes | total duration, µs | average, µs./req. |\n";
    for(int i = 0;i < clientCount;i ++)
    {
        stream << "|" << std::setw(5) << std::right << i
               << " |" << std::setw(12) << std::right << results[i].requests
               << " |" << std::setw(13) << std::right << results[i].total_bytes
               << " |" << std::setw(19) << std::right << results[i].total_duration
               << " |" << std::setw(18) << std::right << (results[i].total_duration / clientCount) << " |\n";
    }
    std::cout << stream.str();

    return 0;
}
