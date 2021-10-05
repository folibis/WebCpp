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

#ifndef WEBCPP_IERROR_H
#define WEBCPP_IERROR_H

#define NO_ERROR 0
#define ERROR (-1)

#include <string>


namespace WebCpp
{

class IErrorable
{
public:
    inline std::string &GetLastError() { return m_lastError; }
    inline void SetLastError(const std::string &error, int errorCode = NO_ERROR) { m_lastError = error; m_lastErrorCode = errorCode; }
    inline int &GetLastErrorCode() { return m_lastErrorCode; }
    inline void ClearError() { m_lastError = ""; m_lastErrorCode = NO_ERROR; }

private:
    std::string m_lastError = "";
    int m_lastErrorCode = NO_ERROR;
};

}

#endif // WEBCPP_IERROR_H
