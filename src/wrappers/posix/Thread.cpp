#include <cstring>
#include <stdexcept>
#include "Thread.h"


using namespace WebCpp;

Thread::Thread()
{

}

bool Thread::Create(ThreadRoutime routime, void *arg)
{
    try
    {
        if(pthread_create(&m_thread, nullptr, routime, arg) != 0)
        {
            throw std::runtime_error(std::string("thread creation error: ") + strerror(errno));
        }

        return true;
    }
    catch(const std::runtime_error &err)
    {
        SetLastError(err.what());
    }
    catch(...)
    {
        SetLastError("thread creation error");
    }

    return false;
}
