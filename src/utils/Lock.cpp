#include "Lock.h"

using namespace WebCpp;

Lock::Lock(pthread_mutex_t &mutex, bool tryLock) : m_mutex(mutex)
{
    if(tryLock)
    {
        if(pthread_mutex_trylock(&mutex) != 0)
        {
            m_successful = false;
        }
    }
    else
    {
        pthread_mutex_lock(&mutex);
    }
}

Lock::~Lock()
{
    pthread_mutex_unlock(&m_mutex);
}
