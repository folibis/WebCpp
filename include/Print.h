/**************************************************************************
/// brief The Print class
/// author ruslan@muhlinin.com
/// date August 1, 2021
/// details The class that allows conditional printing to console
**************************************************************************/

#ifndef PRINT_H
#define PRINT_H

#include <iostream>


namespace WebCpp
{

class Print
{
public:
    Print();

    template<typename T>
    Print& operator<<(const T &t)
    {
        if(Print::AllowPrint)
        {
            std::cout << t;
        }

        return *this;
    }

    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
    typedef std::ostream& (*StandardEndLine)(CoutType&);
    Print& operator<<(StandardEndLine manip)
    {
        if(Print::AllowPrint)
        {
            manip(std::cout);
        }
        return *this;
    }

    static Print& endl(Print& stream)
    {
        if(Print::AllowPrint)
        {
            std::cout << std::endl;
        }
        return stream;
    }

    static bool AllowPrint;
};

}

#endif // PRINT_H
