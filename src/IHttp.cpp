#include "common_webcpp.h"
#include "defines_webcpp.h"
#include "StringUtil.h"
#include "IHttp.h"

using namespace WebCpp;


std::string Http::Protocol2String(Protocol protocol)
{
    switch(protocol)
    {
        case Protocol::HTTP: return "HTTP";
        case Protocol::HTTPS: return "HTTPS";
        default: break;
    }

    return "";
}

Http::Protocol Http::String2Protocol(const std::string &str)
{
    std::string s = str;
    StringUtil::ToUpper(s);

    switch(_(s.c_str()))
    {
        case _("HTTP"): return Protocol::HTTP;
        case _("HTTPS"): return Protocol::HTTPS;
        defaut: break;
    }

    return Protocol::Undefined;
}

Http::Method Http::String2Method(const std::string &str)
{
    std::string s = str;
    StringUtil::ToUpper(s);

    switch(_(s.c_str()))
    {
        case _("OPTIONS"): return Http::Method::OPTIONS;
        case _("GET"):     return Http::Method::GET;
        case _("HEAD"):    return Http::Method::HEAD;
        case _("POST"):    return Http::Method::POST;
        case _("PUT"):     return Http::Method::PUT;
        case _("DELETE"):  return Http::Method::DELETE;
        case _("TRACE"):   return Http::Method::TRACE;
        case _("CONNECT"): return Http::Method::CONNECT;
        default:
            break;
    }

    return Http::Method::Undefined;
}

std::string Http::Method2String(Http::Method method)
{
    switch(method)
    {
        case Http::Method::OPTIONS: return "OPTIONS";
        case Http::Method::GET:     return "GET";
        case Http::Method::HEAD:    return "HEAD";
        case Http::Method::POST:    return "POST";
        case Http::Method::PUT:     return "PUT";
        case Http::Method::DELETE:  return "DELETE";
        case Http::Method::TRACE:   return "TRACE";
        case Http::Method::CONNECT: return "CONNECT";
        default:
            break;
    }

    return "";
}
