#include "Session.h"
#include "Request.h"


using namespace WebCpp;

Session::Session(int connID, const std::string &remote):
    request(new Request()),
    authProvider(AuthProvider::Type::Server)
{
    this->remote = remote;
    request->SetConnectionID(connID);
    request->SetRemote(remote);
    request->SetSession(this);
    readyForDispatch = false;
}
