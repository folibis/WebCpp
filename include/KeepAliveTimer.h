/**************************************************************************
/// @brief The KeepAliveTimer class
/// @author ruslan@muhlinin.com
/// @date August 1, 2021
/// @details The class that handles multiple timers
**************************************************************************/

#ifndef KEEP_ALIVE_TIMER_H
#define KEEP_ALIVE_TIMER_H

#include <functional>
#include <vector>
#include <inttypes.h>
#include <PeriodicalTask.h>

namespace WebCpp
{

class KeepAliveTimer final
{
public:
    ~KeepAliveTimer();
    static void run();
    static void stop();
    static void SetCallback(std::function<void(int)> callback);
    static void SetTimer(uint32_t delay, int connID);

protected:
    static void *task(bool *);

private:
    struct Timer
    {
        int connID;
        int ticks;
        int remain;
    };

    static std::function<void(int)> m_callback;
    static PeriodicalTask m_task;
    static std::vector<Timer> m_timers;
    static pthread_mutex_t m_mutex;
};

}

#endif // KEEP_ALIVE_TIMER_H
