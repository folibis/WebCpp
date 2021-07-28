/**************************************************************************
/// brief The Lock class
/// author ruslan@muhlinin.com
/// date July 28, 2021
/// details The thread lock class with automatic releasing
**************************************************************************/

#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>

namespace WebCpp
{

class Lock
{
public:
    explicit Lock(pthread_mutex_t &mutex, bool tryLock = false);
    ~Lock();
    Lock(const Lock &other) = delete;
    Lock & operator=(const Lock &other) = delete;
    Lock(Lock &&other) = delete;
    Lock & operator=(Lock &&other) = delete;

    bool IsSuccessful() { return m_successful; }

private:
    pthread_mutex_t &m_mutex;
    bool m_successful = true;
};

}

#endif // LOCK_H
