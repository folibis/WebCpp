#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>


namespace WebCpp
{

class Response
{
public:
    Response();
    void SetHeader(const std::string &name, const std::string &value);
    void Write(const std::string &str);
};

}

#endif // RESPONSE_H
