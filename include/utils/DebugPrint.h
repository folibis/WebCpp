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

#ifndef WEBCPP_DEBUG_PRINT_H
#define WEBCPP_DEBUG_PRINT_H

#include <iostream>


namespace WebCpp
{

class DebugPrint
{
public:
    DebugPrint();

    template<typename T>
    DebugPrint& operator<<(const T &t)
    {
        if(DebugPrint::AllowPrint)
        {
            std::cout << t;
        }

        return *this;
    }

    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
    typedef std::ostream& (*StandardEndLine)(CoutType&);
    DebugPrint& operator<<(StandardEndLine manip)
    {
        if(DebugPrint::AllowPrint)
        {
            manip(std::cout);
        }
        return *this;
    }

    static DebugPrint& endl(DebugPrint& stream)
    {
        if(DebugPrint::AllowPrint)
        {
            std::cout << std::endl;
        }
        return stream;
    }

    static bool AllowPrint;
};

}

#endif // WEBCPP_DEBUG_PRINT_H
