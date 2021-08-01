/************************************************************************
/// \brief The PeriodicalTask class
/// \author ruslan@muhlinin.com
/// \date August 1, 2021
/// \details The class that runs periodically tasks in a separate thread
************************************************************************/

#ifndef PERIODICALTASK_H
#define PERIODICALTASK_H

#include <pthread.h>
#include <functional>
#include <unistd.h>

namespace WebCpp
{

class PeriodicalTask
{
public:
    PeriodicalTask();
    void SetFunction(const std::function<void *(bool *)> &func);
    void SetFinishFunction(const std::function<void(void *)> &func);
    void Start();
    void Stop();
    void StopNoWait();
    void Wait() const;
    bool IsRunning() const { return m_isRunning; }

protected:
    static void *StartThread(void *cls);
    void SetStop();

private:
    pthread_t m_thread;
    std::function<void *(bool *)> m_func = nullptr;
    std::function<void(void *)> m_funcFinish = nullptr;
    bool m_isRunning = false;
};

}

#endif // PERIODICALTASK_H
