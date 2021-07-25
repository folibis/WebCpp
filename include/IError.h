/**************************************************************************
/// \brief The IError class
/// \author ruslan@muhlinin.com
/// \date July 25, 2021
/// \details The error handling interface
**************************************************************************/

#ifndef IERROR_H
#define IERROR_H

#define NO_ERROR 0
#define ERROR (-1)

#include <string>


namespace WebCpp
{

class IError
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

#endif // IERROR_H
