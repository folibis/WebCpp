/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#ifndef WEBCPP_HTTPCLIENT_H
#define WEBCPP_HTTPCLIENT_H

#include <memory>
#include "IErrorable.h"
#include "IRunnable.h"
#include "IHttp.h"
#include "ICommunicationClient.h"
#include "HttpConfig.h"
#include "Request.h"
#include "Response.h"
#include "Url.h"


namespace WebCpp {

class HttpClient: public IErrorable, public IRunnable
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
    virtual ~HttpClient();
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

protected:
    void OnDataReady(const ByteArray &data);
    void OnClosed();
    bool InitConnection(const Url &url);
    void SetState(State state);

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

#endif // WEBCPP_HTTPCLIENT_H
