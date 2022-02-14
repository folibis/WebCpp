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

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.2.2

#ifndef WEBCPP_URL_H
#define WEBCPP_URL_H

#include <string>
#include <map>

#define DEFAULT_PORT 80
#define DEFAULT_SECURED_PORT 443


namespace WebCpp
{

class Url
{
public:
    enum class Scheme
    {
        Undefined,
        HTTP,
        HTTPS,
        WS,
        WSS,
    };

    Url(const std::string &url);
    Url();
    bool Parse(const std::string &url, bool full = true);
    std::string ToString(bool full = true) const;
    bool IsInitiaized() const;
    void Clear();

    Scheme GetScheme() const;
    void SetScheme(Scheme scheme);

    std::string GetUser() const;
    void SetUser(const std::string &value);

    std::string GetHost() const;
    void SetHost(const std::string &value);

    int GetPort() const;
    void SetPort(int value);

    std::string GetPath() const;
    std::string GetNormalizedPath() const;
    void SetPath(const std::string &value);

    std::string GetQueryValue(const std::string &name) const;
    void SetQueryValue(const std::string &name, const std::string &value);
    std::string Query2String() const;
    bool HasQuery() const;

    std::string GetFragment() const;
    void SetFragment(const std::string &value);

    size_t GetOriginalSize() const;

    static Scheme String2Scheme(const std::string &str);
    static std::string Scheme2String(Scheme scheme);

protected:
    bool ParseAuthority(const std::string &authority);
    bool ParsePath(const std::string &path);
    bool ParseQuery(const std::string &query);
    bool IsValid() const;

private:
    bool m_initiaized = false;
    Scheme m_scheme = Scheme::Undefined;
    std::string m_user;
    std::string m_host;
    int m_port = DEFAULT_PORT;
    std::string m_path;
    std::map<std::string,std::string> m_query;
    std::string m_fragment;
    size_t m_originalSize = 0;
};

}

#endif // WEBCPP_URL_H
