#include "Session.h"


using namespace WebCpp;

Session::Session()
{

}

bool Session::AddNewSession(int connID, const std::string &remote)
{
    auto it = m_sesions.find(connID);
    if(it == m_sesions.end())
    {
        m_sesions.insert(std::pair<int, SessionItem>(connID, SessionItem(connID, remote)));
        return true;
    }

    return false;
}

bool Session::AppendData(int connID, const ByteArray &data)
{
    auto it = m_sesions.find(connID);
    if(it != m_sesions.end())
    {
        auto &session = it->second;

        session.data.insert(session.data.end(), data.begin(), data.end());
        if(session.request == nullptr)
        {
            session.request.reset(new Request(connID, session.remote));
        }
        return true;
    }

    return false;
}

bool Session::Process()
{
    bool retval = false;
    ClearError();

    for(auto& it: m_sesions)
    {
        auto &session = it.second;
        if(session.request != nullptr && session.data.size() > 0)
        {
            if(session.request->Parse(session.data))
            {
                size_t size = session.request->GetRequestSize();
                if(session.data.size() >= size)
                {
                    session.readyForDispatch = true;
                    session.data.clear();
                    retval = true;
                    break;
                }
            }
            else
            {
                SetLastError("parsing error: " + session.request->GetLastError());
            }
        }
    }

    return retval;
}

std::unique_ptr<Request> Session::GetReadyRequest()
{
    for(auto& it: m_sesions)
    {
        auto &session = it.second;
        if(session.readyForDispatch == true)
        {
            session.readyForDispatch = false;
            return std::unique_ptr<Request>(std::move(session.request));
        }
    }

    return nullptr;
}

bool Session::RemoveSession(int connID)
{
    auto it = m_sesions.find(connID);
    if(it == m_sesions.end())
    {
        m_sesions.erase(it);
        return true;
    }

    return false;
}

bool Session::IsEmpty() const
{
    return m_sesions.empty();
}

bool Session::InitAuth()
{
    try
    {
        auto &list = IAuth::Get();
        for(auto &pair: list)
        {
            auto ptr = pair.second();
            if(ptr != nullptr)
            {
                if(ptr->Init() == true)
                {
                    m_auth.push_back(std::unique_ptr<IAuth>(ptr));
                }
            }
        }
    }
    catch(...) { m_auth.clear(); return false; }

    return true;
}
