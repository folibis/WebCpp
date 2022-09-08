#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <map>
#include <memory>
#include "common_webcpp.h"
#include "IErrorable.h"
#include "Request.h"
#include "AuthProvider.h"
#include "Session.h"


namespace WebCpp
{

class SessionManager : public IErrorable
{
public:
    SessionManager();
    bool AddNewSession(int connID, const std::string &remote);
    bool AppendData(int connID, const ByteArray &data);
    bool Process();
    std::unique_ptr<Request> GetReadyRequest();
    bool RemoveSession(int connID);
    bool IsEmpty() const;
private:


    std::map<int, Session> m_sesions;
};

}

#endif // SESSIONMANAGER_H
