#include "RouteHttp.h"

using namespace WebCpp;

RouteHttp::RouteHttp(const std::string &path, HttpHeader::Method method) :
    Route(path, method)
{

}

bool RouteHttp::SetFunction(const RouteHttp::RouteFunc &f)
{
    m_func = f;
    return true;
}

const RouteHttp::RouteFunc &RouteHttp::GetFunction() const
{
    return m_func;
}
