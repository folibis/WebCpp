#include "Lock.h"

using namespace WebCpp;

Lock::Lock(pthread_mutex_t &mutex, bool tryLock) : m_mutex(mutex)
{
    m_locked = true;
    if(tryLock)
    {
        if(pthread_mutex_trylock(&mutex) != 0)
        {
            m_successful = false;
            m_locked = false;
        }
    }
    else
    {
        if(pthread_mutex_lock(&mutex) != 0)
        {
            m_locked = false;
        }
    }
}

void Lock::Unlock()
{
    if(m_locked)
    {
        pthread_mutex_unlock(&m_mutex);
        m_locked = false;
    }
}

Lock::~Lock()
{
    Unlock();
}
