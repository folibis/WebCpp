/**************************************************************************
/// @brief The GlobalTimer class
/// @author ruslan@muhlinin.com
/// @date August 1, 2021
/// @details The class that handles multiple timers
**************************************************************************/

#ifndef GLOBAL_TIMER_H
#define GLOBAL_TIMER_H

#include <functional>
#include <vector>
#include <inttypes.h>
#include <PeriodicalTask.h>

namespace WebCpp
{

class GlobalTimer final
{
public:
    ~GlobalTimer();
    static void run();
    static void stop();
    static uint32_t addCallback(uint32_t delay, std::function<void (void)> callback);
    static void updateDelay(uint32_t pos, uint32_t delay);

protected:
    static void *task(bool *);

private:
    struct Timer
    {
        uint32_t pos;
        int ticks;
        int remain;
        std::function<void (void)> callback;
    };

    static uint32_t m_pos;
    static PeriodicalTask m_task;
    static std::vector<Timer> m_timers;
    static pthread_mutex_t m_mutex;
};

}

#endif // GLOBAL_TIMER_H
