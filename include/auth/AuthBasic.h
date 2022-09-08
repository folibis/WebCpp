#ifndef AUTHBASIC_H
#define AUTHBASIC_H

#include "AuthFactory.h"

#define BASIC_DEFAULT_REALM "WEBCPP"


namespace WebCpp {

class AuthBasic : public IAuth
{
    IAUTH_DECL
public:
    AuthBasic();
    bool Init() override;
    bool ParseFromClient(const std::string &token) override;
    bool ParseFromServer(const std::string &token) override;
    std::string GetChallenge() const override;
    std::string GetSchemeName() const override;
    bool AddAuthHeaders(Request &request) override;

    std::string m_realm = BASIC_DEFAULT_REALM;
    std::string m_charset = "";
};

}

#endif // AUTHBASIC_H
