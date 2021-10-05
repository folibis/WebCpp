#include "ThreadWorker.h"

using namespace WebCpp;

ThreadWorker::ThreadWorker()
{
    m_isRunning = false;
}

void ThreadWorker::SetFunction(const std::function<void *(bool *running)> &func)
{
    m_func = func;
}

void ThreadWorker::SetFinishFunction(const std::function<void (void *)> &func)
{
    m_funcFinish = func;
}

void ThreadWorker::Start()
{
    if(m_isRunning)
        return;

    ClearError();
    m_isRunning = true;

    if(pthread_create( &m_thread, nullptr, ThreadWorker::StartThread, this) != 0)
    {
        SetLastError("failed to starting a thread");
    }
}

void ThreadWorker::Stop()
{
    if(m_isRunning)
    {
        m_isRunning = false;
        pthread_join(m_thread, nullptr);
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
        res = instance->m_func(&(instance->m_isRunning));
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
