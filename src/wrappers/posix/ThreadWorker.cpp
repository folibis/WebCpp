#include "ThreadWorker.h"

using namespace WebCpp;

ThreadWorker::ThreadWorker()
{
    m_isRunning = false;
}

void ThreadWorker::SetFunction(const std::function<ThreadRoutine> &func)
{
    m_func = func;
}

void ThreadWorker::SetFinishFunction(const std::function<ThreadFinishRoutine> &func)
{
    m_funcFinish = func;
}

bool ThreadWorker::Start()
{
    if(m_isRunning)
    {
        return true;
    }

    ClearError();
    m_isRunning = true;

    if(pthread_create(&m_thread, nullptr, ThreadWorker::StartThread, this) != 0)
    {
        SetLastError("failed to starting a thread");
        return false;
    }

    return true;
}

void ThreadWorker::Stop(bool wait)
{
    if(m_isRunning)
    {
        m_isRunning = false;
        if(wait)
        {
            pthread_join(m_thread, nullptr);
        }
    }
}

void ThreadWorker::StopNoWait()
{
    m_isRunning = false;
}

void ThreadWorker::Wait() const
{
    if(m_isRunning)
    {
        pthread_join(m_thread, nullptr);
    }
}

void *ThreadWorker::StartThread(void *cls)
{
    void *res = nullptr;
    ThreadWorker *instance = static_cast<ThreadWorker *>(cls);
    if(instance->m_func)
    {
        res = instance->m_func(instance->m_isRunning);
    }
    instance->SetStop();
    if(instance->m_funcFinish)
    {
        instance->m_funcFinish(res);
    }
    return res;
}

void ThreadWorker::SetStop()
{
    m_isRunning = false;
}
