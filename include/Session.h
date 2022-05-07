#ifndef SESSION_H
#define SESSION_H

#include <map>
#include "IErrorable.h"
#include "Request.h"


namespace WebCpp
{

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
};

}

#endif // SESSION_H
