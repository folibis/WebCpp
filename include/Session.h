#ifndef SESSION_H
#define SESSION_H

#include "common_webcpp.h"
#include "AuthProvider.h"


namespace WebCpp
{
class Request;
struct Session
{
public:
    Session(int connID, const std::string &remote);

    ByteArray data;
    std::unique_ptr<Request> request;
    bool readyForDispatch;
    std::string remote;
    AuthProvider authProvider;
};

}

#endif // SESSION_H
