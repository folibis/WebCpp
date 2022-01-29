#include "Mutex.h"


using namespace WebCpp;

pthread_mutex_t* Mutex::GetMutex()
{
    return &m_writeMutex;
}
