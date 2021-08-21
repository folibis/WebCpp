#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <memory>
#include <deque>
#include "HttpConfig.h"
#include "Route.h"
#include "IError.h"
#include "IRunnable.h"
#include "Request.h"
#include "ICommunicationServer.h"


namespace WebCpp
{

class WebSocketServer: public IError, public IRunnable
{
public:
    enum class Protocol
    {
        Undefined = 0,
        WS,
        WSS,
    };
    WebSocketServer();
    WebSocketServer(const WebSocketServer& other) = delete;
    WebSocketServer& operator=(const WebSocketServer& other) = delete;
    WebSocketServer(WebSocketServer&& other) = delete;
    WebSocketServer& operator=(WebSocketServer&& other) = delete;

    bool Init(WebCpp::HttpConfig config);
    bool Run() override;
    bool Close(bool wait = true) override;
    bool WaitFor() override;

    void SetWebSocketRequestFunc(const Route::RouteFunc &callback);
    void Data(const std::function<ByteArray(const HttpHeader& request, const ByteArray &data)>& func);

    Protocol GetProtocol() const;

    static Protocol String2Protocol(const std::string &str);
    static std::string Protocol2String(Protocol protocol);

    std::string ToString() const;

protected:
    struct RequestData
    {
        RequestData(int connID, ByteArray& data)
        {
            this->connID= connID;
            this->data = std::move(data);
            readyForDispatch = false;
            handshake = false;
        }
        int connID;
        HttpHeader header;
        ByteArray data;
        bool handshake;
        bool readyForDispatch;
    };

    void OnConnected(int connID, const std::string& remote);
    void OnDataReady(int connID, std::vector<char> &data);
    void OnClosed(int connID);

    static void* RequestThreadWrapper(void *ptr);
    void* RequestThread();
    void SendSignal();
    void WaitForSignal();
    void PutToQueue(int connID, ByteArray &data);
    bool IsQueueEmpty();
    bool CheckDataFullness();
    void ProcessRequests();
    void RemoveFromQueue(int connID);
    bool ProcessRequest(const Request &request);
    bool CheckWsHeader(RequestData& requestData);
    bool CheckWsFrame(RequestData &requestData);
    bool ProcessWsRequest(int connID, const HttpHeader &request, const ByteArray &data);

private:
#pragma pack(push, 1)
    struct Flag1
    {
        uint8_t FIN:  1;
        uint8_t RSV1: 1;
        uint8_t RSV2: 1;
        uint8_t RSV3: 1;
        uint8_t opcode: 4;
    };
    struct Flag2
    {
        uint8_t Mask:  1;
        uint8_t PayloadLen: 7;
    };

    struct WebSocketHeader
    {
        Flag1 flags1;
        Flag2 flags2;
    };
    struct WebSocketHeaderLength2
    {
        uint16_t length;
    };
    struct WebSocketHeaderLength3
    {
        uint64_t length;
    };
    struct WebSocketHeaderMask
    {
        uint32_t mask;
    };
#pragma pack(pop)

    std::shared_ptr<ICommunicationServer> m_server = nullptr;
    Protocol m_protocol = Protocol::Undefined;
    pthread_t m_requestThread;
    pthread_mutex_t m_queueMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t m_signalMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_signalCondition = PTHREAD_COND_INITIALIZER;
    bool m_requestThreadRunning = false;
    std::deque<RequestData> m_requestQueue;
    HttpConfig m_config;
    Route::RouteFunc m_webSocketRequest = nullptr;
    std::function<ByteArray(const HttpHeader& request, const ByteArray& data)> m_dataFunc;
};

}

#endif // WEBSOCKETSERVER_H
