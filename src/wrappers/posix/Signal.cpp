#include "Signal.h"


using namespace WebCpp;

Signal::Signal()
{

}

void Signal::Fire()
{
    pthread_cond_signal(&m_signalCondition);
}

void Signal::Wait(Mutex &mutex)
{
    pthread_cond_wait(& m_signalCondition, mutex.GetMutex());
}
