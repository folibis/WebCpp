#ifndef AUTHPROVIDER_H
#define AUTHPROVIDER_H

#include <map>
#include <memory>
#include "IAuth.h"


namespace WebCpp {

class AuthProvider
{
public:
    enum class Type
    {
        Server,
        Client,
    };

    AuthProvider(Type type);
    bool Init(const std::string &str = "");
    bool Parse(const std::string &str);
    bool Check(const std::string &str);
    bool IsInitialized() const;
    IAuth* GetPreferred() const;
    const std::vector<std::unique_ptr<IAuth>> &Get() const;
    void Clear();

protected:
    bool InitServer(const std::string &str);
    bool InitClient(const std::string &str);
    bool ParseFromServer(const std::string &str);
    bool ParseFromClient(const std::string &str);
    bool CheckFromServer(const std::string &str);
    bool CheckFromClient(const std::string &str);
    void ClearPreferred();

private:
    Type m_type;
    bool m_initialized = false;
    std::vector<std::unique_ptr<IAuth>> m_auth;
};

}

#endif // AUTHPROVIDER_H
