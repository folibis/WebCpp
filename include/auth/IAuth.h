#ifndef IAUTH_H
#define IAUTH_H

#include <vector>
#include <map>
#include <string>


namespace WebCpp {

class Request;
class IAuth
{
public:
    virtual bool Init() = 0;
    virtual bool ParseFromClient(const std::string &token) = 0;
    virtual bool ParseFromServer(const std::string &token) = 0;
    virtual std::string GetChallenge() const = 0;
    virtual std::string GetSchemeName() const = 0;
    virtual bool AddAuthHeaders(Request &request) = 0;

    virtual std::string GetUser() const;
    virtual bool SetUser(const std::string &user);
    virtual std::string GetPassword() const;
    virtual bool SetPassword(const std::string &password);
    virtual void SetPreferred(bool value);
    virtual bool IsPreferred() const;

protected:
    std::string m_user = "";
    std::string m_password = "";
    bool m_preferred = false;
};

}

#endif // IAUTH_H
