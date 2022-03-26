#ifndef EXAMPLE_COMMON_H
#define EXAMPLE_COMMON_H

#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include "defines_webcpp.h"
#include "FileSystem.h"

#define PUB PUBLIC_DIR
#define DEFAULT_HTTP_PORT 8080
#define DEFAULT_WS_PORT 8081
#define DEFAULT_HTTP_PROTOCOL "HTTP"
#define DEFAULT_WS_PROTOCOL "WS"
#define SSL_CERT "~/.ssh/server.cert"
#define SSL_KEY "~/.ssh/server.key"


class CommandLine
{
public:
    static CommandLine Parse(int argc, char **argv)
    {
        std::map<std::string,std::string> retval;
        std::string name;
        std::string value;
        std::string token;

        for(int i = 1;i < argc;i ++)
        {
            token = argv[i];
            if(token.at(0) == '-')
            {
                if(!name.empty())
                {
                    retval[name] = value;
                }
                name = token;
            }
            else
            {
                value = token;
                if(!name.empty())
                {
                    retval[name] = value;
                }
                name = "";
                value = "";
            }
        }
        if(!name.empty())
        {
            retval[name] = value;
        }

        return CommandLine(argv[0], retval);
    }

    std::string Get(const std::string &name)
    {
        auto it = m_values.find(name);
        if(it == m_values.end())
        {
            return "";
        }

        return it->second;
    }


    bool Set(const std::string &name, std::string &value)
    {
        auto it = m_values.find(name);
        if(it != m_values.end())
        {
            value = it->second;
            return true;
        }

        return false;
    }

    bool Exists(const std::string &name) const
    {
        auto it = m_values.find(name);
        return (it != m_values.end());
    }

    bool Empty() const
    {
        return (m_values.size() > 0);
    }

    void PrintUsage(bool ws = true, bool http = false, const std::vector<std::string> &adds = std::vector<std::string>())
    {
        std::cout << "Usage: " << WebCpp::FileSystem::ExtractFileName(m_exe) << " [options]" << std::endl;
        std::cout << "where [options] are:" << std::endl;
        if(http)
        {
            std::cout << "\t-ph: HTTP port" << std::endl;
            std::cout << "\t-rh: HTTP protocol [HTTP,HTTPS]" << std::endl;
        }
        if(ws)
        {
            std::cout << "\t-pw: WebSocket port" << std::endl;
            std::cout << "\t-rw: WebSocket protocol [WS,WSS]" << std::endl;
        }
        for(auto &a: adds)
        {
            std::cout << "\t" << a << std::endl;
        }
        std::cout << "\t-v: verbose output" << std::endl;
        std::cout << "\t-h: print this message and exit" << std::endl;
    }

private:
    CommandLine(const std::string &exe, const std::map<std::string,std::string> &args)
    {
        m_exe = exe;
        m_values = args;
    }

protected:
    std::string m_exe;
    std::map<std::string,std::string> m_values;
};

#endif // EXAMPLE_COMMON_H
