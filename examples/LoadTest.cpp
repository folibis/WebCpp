#include <signal.h>
#include <string>
#include <unistd.h>
#include <chrono>
#include <sstream>
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
#define DEFAULT_DELAY 100
#define TEST_COUNT 100

size_t clientCount = DEFAULT_CLIENT_COUNT;
std::string resource = DEFAULT_RESOURCE;
long delay = DEFAULT_DELAY;
int testCount = TEST_COUNT;
bool g_running = true;
static int g_id = 0;


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
        WebCpp::HttpConfig config;
        config.SetTempFile(true);
        config.SetMaxBodyFileSize(10_Mb);
        std::chrono::steady_clock::time_point start;
        std::chrono::steady_clock::time_point end;
        long total_duration = 0;

        WebCpp::DebugPrint::AllowPrint = true;

        if(httpCient.Init(config))
        {
            httpCient.SetResponseCallback([&httpCient,id,&start,&end,&total_duration](const WebCpp::Response &response) -> bool
            {
                end = std::chrono::steady_clock::now();
                long dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                total_duration += dur;
                std::stringstream stream;
                stream
                        << id
                        << ": response code: "
                        << response.GetResponseCode()
                        << ", size:  " << response.GetHeader().GetRequestSize()
                        << ", duration: " << dur << " µs."
                        << std::endl;
                std::cout << stream.str();
                //StringUtil::PrintHex(response.GetBody());
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
            std::stringstream stream;
            stream
                    << "thread " << id << ": "
                    << cnt << " request"
                    << ", total: " << total_duration << " µs."
                    << ", average: " << (cnt > 0 ? (total_duration / cnt) : 0) << " µs./req."
                    << std::endl;
            std::cout << stream.str();
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

    std::vector<WebCpp::ThreadWorker> workers;
    workers.resize(clientCount);
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

    return 0;
}
