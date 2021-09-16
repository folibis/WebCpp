#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <memory>
#include "IError.h"
#include "IRunnable.h"
#include "IHttp.h"
#include "ICommunicationClient.h"
#include "HttpConfig.h"
#include "Request.h"
#include "Response.h"
#include "Url.h"


namespace WebCpp {

class HttpClient: public IError, public IRunnable
{
public:
    enum class State
    {
        Undefined = 0,
        Initialized,
        Connected,
        DataSent,
        DataReady,
        Closed,
    };

    HttpClient();
    HttpClient(const HttpClient& other) = delete;
    HttpClient& operator=(const HttpClient& other) = delete;
    HttpClient(HttpClient&& other) = delete;
    HttpClient& operator=(HttpClient&& other) = delete;

    bool Init() override;
    bool Init(const WebCpp::HttpConfig& config);
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;

    bool Open(Request &request);
    bool Open(Http::Method method, const std::string &url, const std::map<std::string, std::string> &headers = {});
    void SetResponseCallback(const std::function<bool(const Response&)> &func);
    void SetStateCallback(const std::function<void(State)> &func);
    void SetProgressCallback(const std::function<void(size_t,size_t)> &func);
    State GetState() const;
    void FireStateChanged() const;

protected:
    void OnDataReady(const ByteArray &data);
    void OnClosed();
    bool InitConnection(const Url &url);

private:
    std::shared_ptr<ICommunicationClient> m_connection = nullptr;
    HttpConfig m_config;
    State m_state;
    ByteArray m_buffer;
    std::function<void(State)> m_stateCallback = nullptr;
    std::function<bool(const Response&)> m_responseCallback = nullptr;
    std::function<void(size_t,size_t)> m_progressCallback = nullptr;

};

}

#endif // HTTPCLIENT_H
