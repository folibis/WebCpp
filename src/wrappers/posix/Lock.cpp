#include "Lock.h"
#include "pthread.h"


using namespace WebCpp;

Lock::Lock(Mutex &mutex) : m_mutex(mutex)
{
    m_locked = true;

    if(pthread_mutex_lock(m_mutex.GetMutex()) != 0)
    {
        m_locked = false;
    }
}

void Lock::Unlock()
{
    if(m_locked)
    {
        pthread_mutex_unlock(m_mutex.GetMutex());
        m_locked = false;
    }
}

Lock::~Lock()
{
    Unlock();
}
