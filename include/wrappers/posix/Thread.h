#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include "IErrorable.h"


namespace WebCpp
{

class Thread: public IErrorable
{
public:
    using ThreadRoutime = void *(*) (void *);

    Thread();
    bool Create(ThreadRoutime routime, void *arg);

private:
    pthread_t m_thread;
};

}

#endif // THREAD_H
