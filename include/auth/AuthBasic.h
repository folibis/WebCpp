#ifndef AUTHBASIC_H
#define AUTHBASIC_H

#include "IAuth.h"

namespace WebCpp {

class AuthBasic : public IAuth
{
    IAUTH_DECL
public:
    AuthBasic();
    bool Init() override;
    bool Parse(const Request &request) override;
    std::string GetChallenge() const override;
};

}

#endif // AUTHBASIC_H
