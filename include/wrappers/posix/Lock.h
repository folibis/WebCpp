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

#ifndef WEBCPP_LOCK_H
#define WEBCPP_LOCK_H

#include "Mutex.h"

namespace WebCpp
{

class Lock
{
public:
    explicit Lock(Mutex &mutex);
    ~Lock();
    Lock(const Lock &other) = delete;
    Lock & operator=(const Lock &other) = delete;
    Lock(Lock &&other) = delete;
    Lock & operator=(Lock &&other) = delete;
    bool IsSuccessful() { return m_successful; }
    void Unlock();
private:
    Mutex &m_mutex;
    bool m_locked = true;
    bool m_successful = true;
};

}

#endif // WEBCPP_LOCK_H
