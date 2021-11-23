#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <string>
#include <functional>
#include <memory>
#include "IErrorable.h"
#include "IRunnable.h"
#include "ICommunicationClient.h"
#include "HttpConfig.h"
#include "Request.h"
#include "ResponseWebSocket.h"


namespace WebCpp
{

class WebSocketClient: public IErrorable, public IRunnable
{
public:
    enum class State
    {
        Undefined = 0,
        Initialized,
        Connected,
        HandShakeSent,
        HandShake,
        BinaryMessage,
        Closed,
    };

    WebSocketClient();
    bool Init() override;
    bool Init(const WebCpp::HttpConfig& config);
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;
    bool Open(Request &request);
    bool Open(const std::string &address);
    bool SendText(const ByteArray &data);
    bool SendText(const std::string &data);
    bool SendBinary(const ByteArray &data);
    bool SendBinary(const std::string &data);


    void SetOnConnect(const std::function<void(bool)> &callback);
    void SetOnClose(const std::function<void(void)> &callback);
    void SetOnError(const std::function<void(const std::string &)> &callback);
    void SetOnMessage(const std::function<void(ResponseWebSocket &)> &callback);
    void SetProgressCallback(const std::function<void(size_t,size_t)> &callback);
    void SetOnStateChanged(const std::function<void(State)> &callback);

protected:
    void OnDataReady(const ByteArray &data);
    void OnClosed();
    bool InitConnection(const Url &url);
    void SetState(State state);

private:
    std::shared_ptr<ICommunicationClient> m_connection = nullptr;
    HttpConfig m_config;
    std::function<void(bool)> m_connectCallback = nullptr;
    std::function<void(void)> m_closeCallback = nullptr;
    std::function<void(const std::string &)> m_errorCallback = nullptr;
    std::function<void(ResponseWebSocket &)> m_messageCallback = nullptr;
    std::function<void(size_t,size_t)> m_progressCallback = nullptr;
    std::function<void(State)> m_stateCallback = nullptr;
    State m_state = State::Undefined;
    std::string m_key;
    ByteArray m_data;
};

}

#endif // WEBSOCKETCLIENT_H
