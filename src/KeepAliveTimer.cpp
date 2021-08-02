#include "Lock.h"
#include "KeepAliveTimer.h"

#define TICK 100 // msec.


using namespace WebCpp;

std::function<void(int)> KeepAliveTimer::m_callback = nullptr;;
PeriodicalTask KeepAliveTimer::m_task;
std::vector<KeepAliveTimer::Timer> KeepAliveTimer::m_timers;
pthread_mutex_t KeepAliveTimer::m_mutex = PTHREAD_MUTEX_INITIALIZER;

KeepAliveTimer::~KeepAliveTimer()
{
    stop();
}

void KeepAliveTimer::run()
{
    m_task.SetFunction(&KeepAliveTimer::task);
    m_task.Start();
}

void KeepAliveTimer::stop()
{
    m_task.Stop();
}

void KeepAliveTimer::SetCallback(std::function<void (int)> callback)
{
    m_callback = callback;
}

void KeepAliveTimer::SetTimer(uint32_t delay, int connID)
{
    Lock lock(m_mutex);

    for(Timer &timer: m_timers)
    {
        if(timer.connID == connID)
        {
            timer.ticks = delay / TICK;
            timer.remain = timer.ticks;
            return;
        }
    }

    Timer timer;
    timer.connID = connID;
    timer.ticks = delay / TICK;
    timer.remain = timer.ticks;

    m_timers.push_back(std::move(timer));
}

void *KeepAliveTimer::task(bool *running)
{
    while(*running)
    {
        if(!m_timers.empty())
        {
            Lock lock(m_mutex);
            for(auto it = m_timers.begin();it != m_timers.end();++it)
            {
                auto &timer = (*it);
                timer.remain --;
                if(timer.remain == 0)
                {
                    if(m_callback != nullptr)
                    {
                        m_callback(timer.connID);
                    }
                    m_timers.erase(it);
                    break;
                }
            }
        }

        usleep(TICK * 1000);
    }

    return nullptr;
}
