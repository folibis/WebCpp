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
    bool Open(Http::Method method, const std::string &address);
    void SetResponseCallback(const std::function<bool(const Response&)> &func);

protected:
    void OnDataReady(ByteArray &data);
    void OnClosed();
    bool InitConnection(const Url &url);

private:
    std::shared_ptr<ICommunicationClient> m_connection = nullptr;
    HttpConfig m_config;
    ByteArray m_buffer;
    std::function<bool(const Response&)> m_responseCallback = nullptr;
};

}

#endif // HTTPCLIENT_H
