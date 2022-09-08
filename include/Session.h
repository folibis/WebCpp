#ifndef SESSION_H
#define SESSION_H

#include <map>
#include "IErrorable.h"
#include "IAuth.h"


namespace WebCpp
{

class Request;
class Session : public IErrorable
{
public:
    Session();
    bool AddNewSession(int connID, const std::string &remote);
    bool AppendData(int connID, const ByteArray &data);
    bool Process();
    std::unique_ptr<Request> GetReadyRequest();
    bool RemoveSession(int connID);
    bool IsEmpty() const;
    bool InitAuth();

private:
    struct SessionItem
    {
        SessionItem(int connID, const std::string &remote):
            request(new Request())
        {
            this->remote = remote;
            request->SetConnectionID(connID);
            request->SetRemote(remote);
            readyForDispatch = false;
        }
        ByteArray data;
        std::unique_ptr<Request> request;
        bool readyForDispatch;
        std::string remote;
    };

    std::map<int, SessionItem> m_sesions;
    std::vector<std::unique_ptr<IAuth>> m_auth;
};

}

#endif // SESSION_H
