#include"StringUtil.h"
#include "Url.h"


using namespace WebCpp;

Url::Url(const std::string &url)
{
    Parse(url);
}

Url::Url()
{

}

bool Url::Parse(const std::string &url, bool full)
{
    size_t pos = 0, prev = 0;
    std::string temp;

    m_originalSize = url.size();

    if(full)
    {
        pos = url.find(':');
        if(pos == std::string::npos)
        {
            return false;
        }
        temp = std::string(url.begin(),url.begin() + pos);
        m_scheme = String2Scheme(temp);
        if(m_scheme == Url::Scheme::Undefined)
        {
            return false;
        }
        prev = pos + 1;
        pos = url.find("//", prev);
        if(pos != std::string::npos) // authority presents
        {
            prev = pos + 2;
            pos = url.find("/", prev);
            std::string authority;
            if(pos != std::string::npos)
            {
                authority = std::string(url.begin() + prev, url.begin() + pos);
                ParseAuthority(authority);
                prev = pos + 1;
            }
            else
            {
                authority = std::string(url.begin() + prev, url.end());
                ParseAuthority(authority);
                m_initiaized = true;
                return true;
            }
        }
    }

    m_initiaized = ParsePath(std::string(url.begin() + prev, url.end()));

    return m_initiaized;
}

bool Url::ParseAuthority(const std::string &authority)
{
    size_t pos = 0, prev = 0;
    std::string temp;

    pos = authority.find("@", prev); // username presents
    if(pos != std::string::npos)
    {
        m_user = std::string(authority.begin() + prev, authority.begin() + pos);
        prev = pos + 1;
    }
    pos = authority.find(":", prev);
    if(pos != std::string::npos) // port presents
    {
        m_host = std::string(authority.begin() + prev, authority.begin() + pos);
        prev = pos + 1;
        temp = std::string(authority.begin() + prev, authority.end());
        int i;
        if(StringUtil::String2int(temp, i))
        {
            m_port = i;
        }
        else
        {
            return false;
        }
    }
    else
    {
        m_host = std::string(authority.begin() + prev, authority.end());
    }

    return true;
}

bool Url::ParsePath(const std::string &path)
{
    size_t pos = 0, prev = 0;
    std::string temp;

    pos = path.find("?", prev);
    if(pos != std::string::npos) // query presents
    {
        m_path = std::string(path.begin() + prev, path.begin() + pos);
        prev = pos + 1;
        pos = path.find("#", prev); // fragment presents
        if(pos != std::string::npos)
        {
            temp = std::string(path.begin() + prev, path.begin() + pos);
            if(ParseQuery(temp) == false)
            {
                return false;
            }
            m_fragment = std::string(path.begin() + pos + 1, path.end());
        }
        else
        {
            temp = std::string(path.begin() + prev, path.end());
            if(ParseQuery(temp) == false)
            {
                return false;
            }
        }
    }
    else
    {
        m_path = path;
    }

    return true;
}

bool Url::ParseQuery(const std::string &query)
{
    auto q = StringUtil::Split(query, '&');
    for(auto &token: q)
    {
        auto pair = StringUtil::Split(token, '=');
        if(pair.size() == 2)
        {
            StringUtil::UrlDecode(pair[0]);
            StringUtil::UrlDecode(pair[1]);
            m_query[pair[0]] = pair[1];
        }
    }

    return true;
}

std::string Url::Query2String() const
{
    std::string retval;
    for(auto const &entry: m_query)
    {
        std::string name = entry.first;
        std::string value = entry.second;
        StringUtil::UrlEncode(name);
        StringUtil::UrlEncode(value);
        retval += (retval.empty() ? "" : "&") + name + "=" + value;
    }

    return retval;
}

bool Url::HasQuery() const
{
    return (!m_query.empty());
}

bool Url::IsValid() const
{
    return (m_scheme != Url::Scheme::Undefined);
}

std::string Url::ToString(bool full) const
{
    std::string retval;
    if(full)
    {
        retval += Scheme2String(m_scheme) + ":";
        if(!m_host.empty())
        {
            retval += "//";
            if(!m_user.empty())
            {
                retval += "@" + m_user;
            }
            retval += m_host;
            if(m_port != DEFAULT_PORT)
            {
                retval += ":" + std::to_string(m_port);
            }
        }
    }

    retval += "/" + m_path;

    if(m_query.size() > 0)
    {
        retval += "?" + Query2String();
    }

    if(!m_fragment.empty())
    {
        retval += "#" + m_fragment;
    }

    return retval;
}

bool Url::IsInitiaized() const
{
    return m_initiaized;
}

void Url::Clear()
{
    m_initiaized = false;
    m_scheme = Scheme::Undefined;
    m_user = "";
    m_host = "";
    m_port = DEFAULT_PORT;
    m_path = "";
    m_query.clear();
    m_fragment = "";
    m_originalSize = 0;
}

Url::Scheme Url::GetScheme() const
{
    return m_scheme;
}

void Url::SetScheme(Scheme scheme)
{
    m_scheme = scheme;
    m_initiaized = IsValid();
}

std::string Url::GetUser() const
{
    return m_user;
}

void Url::SetUser(const std::string &value)
{
    m_user = value;
}

std::string Url::GetHost() const
{
    return m_host;
}

void Url::SetHost(const std::string &value)
{
    m_host = value;
    m_initiaized = IsValid();
}

int Url::GetPort() const
{
    return m_port;
}

void Url::SetPort(int value)
{
    m_port = value;
}

std::string Url::GetPath() const
{
    return m_path;
}

std::string Url::GetNormalizedPath() const
{
    return (m_path.size() > 0 && m_path.at(0) == '/') ? m_path : '/' + m_path;
}

void Url::SetPath(const std::string &value)
{
    m_path = value;
}

std::string Url::GetQueryValue(const std::string &name) const
{
    auto it = m_query.find(name);
    if(it != m_query.end())
    {
        return it->second;
    }

    return "";
}

void Url::SetQueryValue(const std::string &name, const std::string &value)
{
    m_query[name] = value;
}

std::string Url::GetFragment() const
{
    return m_fragment;
}

void Url::SetFragment(const std::string &value)
{
    m_fragment = value;
}

size_t Url::GetOriginalSize() const
{
    return m_originalSize;
}

Url::Scheme Url::String2Scheme(const std::string &str)
{
    std::string s = str;
    StringUtil::ToLower(s);

    switch(_(s.c_str()))
    {
        case _("http"): return Url::Scheme::HTTP;
        case _("https"): return Url::Scheme::HTTPS;
        case _("ws"): return Url::Scheme::WS;
        case _("wss"): return Url::Scheme::WSS;
        default: break;
    }

    return Url::Scheme::Undefined;
}

std::string Url::Scheme2String(Scheme scheme)
{
    switch(scheme)
    {
        case Url::Scheme::HTTP: return "http";
        case Url::Scheme::HTTPS: return "https";
        case Url::Scheme::WS: return "ws";
        case Url::Scheme::WSS: return "wss";
        default: break;
    }

    return "";
}


