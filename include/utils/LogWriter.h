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

#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <string>
#include <fstream>

#define LOG(S,T) LogWriter::Instance().Write(S,T)


class LogWriter
{
public:
    enum class LogType
    {
        Info = 0,
        Error = 1,
        Access = 2,        
    };

    ~LogWriter();
    static LogWriter& Instance();
    void Write(const std::string &text, LogType type = LogType::Info);

protected:
    LogWriter();    

    bool Open();
    bool Close();
    static std::string LogType2String(LogType type);

private:
    bool m_opened = false;
    std::ofstream m_streams[3];
};

#endif // LOGWRITER_H
