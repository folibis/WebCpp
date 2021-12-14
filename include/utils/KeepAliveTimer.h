/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#ifndef WEBCPP_KEEP_ALIVE_TIMER_H
#define WEBCPP_KEEP_ALIVE_TIMER_H

#include <functional>
#include <vector>
#include <inttypes.h>
#include <ThreadWorker.h>

namespace WebCpp
{

class KeepAliveTimer final
{
public:
    ~KeepAliveTimer();
    static void run();
    static void stop();
    static void SetCallback(std::function<void(int)> callback);
    static void SetTimer(uint32_t delay, int connID);

protected:
    static void *task(bool &);

private:
    struct Timer
    {
        int connID;
        int ticks;
        int remain;
    };

    static std::function<void(int)> m_callback;
    static ThreadWorker m_task;
    static std::vector<Timer> m_timers;
    static pthread_mutex_t m_mutex;
};

}

#endif // WEBCPP_KEEP_ALIVE_TIMER_H
