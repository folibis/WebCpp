#include "PeriodicalTask.h"

using namespace WebCpp;

PeriodicalTask::PeriodicalTask()
{
    m_isRunning = false;
}

void PeriodicalTask::SetFunction(const std::function<void *(bool *running)> &func)
{
    m_func = func;
}

void PeriodicalTask::SetFinishFunction(const std::function<void (void *)> &func)
{
    m_funcFinish = func;
}

void PeriodicalTask::Start()
{
    if(m_isRunning)
        return;

    ClearError();
    m_isRunning = true;

    if(pthread_create( &m_thread, nullptr, PeriodicalTask::StartThread, this) != 0)
    {
        SetLastError("failed to starting a thread");
    }
}

void PeriodicalTask::Stop()
{
    if(m_isRunning)
    {
        m_isRunning = false;
        pthread_join(m_thread, nullptr);
    }
}

void PeriodicalTask::StopNoWait()
{
    m_isRunning = false;
}

void PeriodicalTask::Wait() const
{
    if(m_isRunning)
    {
        pthread_join(m_thread, nullptr);
    }
}

void *PeriodicalTask::StartThread(void *cls)
{
    PeriodicalTask *instance = static_cast<PeriodicalTask *>(cls);
    void *res = instance->m_func(&(instance->m_isRunning));
    instance->SetStop();
    if(instance->m_funcFinish)
    {
        instance->m_funcFinish(res);
    }
    return res;
}

void PeriodicalTask::SetStop()
{
    m_isRunning = false;
}
