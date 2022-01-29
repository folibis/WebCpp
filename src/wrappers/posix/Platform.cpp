#include "Platform.h"
#include "unistd.h"



void WebCpp::SleepMs(uint32_t delay)
{
    usleep(delay * 1000);
}

void WebCpp::Sleep(uint32_t delay)
{
    sleep(delay);
}
