#include "Lock.h"
#include "GlobalTimer.h"


#define TICK 100


using namespace WebCpp;

uint32_t GlobalTimer::m_pos = 0;
PeriodicalTask GlobalTimer::m_task;
std::vector<GlobalTimer::Timer> GlobalTimer::m_timers;
pthread_mutex_t GlobalTimer::m_mutex = PTHREAD_MUTEX_INITIALIZER;

GlobalTimer::~GlobalTimer()
{
    stop();
}

void GlobalTimer::run()
{
    m_task.SetFunction(&GlobalTimer::task);
    m_task.Start();
}

void GlobalTimer::stop()
{
    m_task.Stop();
}

uint32_t GlobalTimer::addCallback(uint32_t delay, std::function<void ()> callback)
{
    Lock lock(m_mutex);

    uint32_t pos = ++GlobalTimer::m_pos;
    Timer timer;
    timer.pos = pos;
    timer.ticks = delay / TICK;
    timer.remain = timer.ticks;
    timer.callback = callback;

    m_timers.push_back(std::move(timer));

    return pos;
}

void GlobalTimer::updateDelay(uint32_t pos, uint32_t delay)
{
    if(pos == 0)
        return;

    Lock lock(m_mutex);

    for(Timer &timer: m_timers)
    {
        if(timer.pos == pos)
        {
            timer.ticks = delay / TICK;
            break;
        }
    }
}

void *GlobalTimer::task(bool *running)
{
    while(*running)
    {
        Lock lock(m_mutex);

        for(Timer &timer: m_timers)
        {
            timer.remain --;
            if(timer.remain == 0)
            {
                timer.callback();
                timer.remain = timer.ticks;
            }
        }

        usleep(TICK * 1000);
    }

    return nullptr;
}
