#ifndef REQUEST_H
#define REQUEST_H

#include <map>
#include "common.h"
#include "HttpConfig.h"
#include "RequestBody.h"
#include "HttpHeader.h"

namespace WebCpp
{

class Request
{
public:
    Request(const HttpConfig& config);
    Request(int connID, const ByteArray &request, HttpHeader &&header, const HttpConfig &config);

    int GetConnectionID() const;        
    const HttpHeader& GetHeader() const;
    const ByteArray &GetData() const;
    const RequestBody& GetRequestBody() const;
    std::string GetArg(const std::string &name) const;
    void SetArg(const std::string &name, const std::string &value);
    bool IsKeepAlive() const;    

protected:
    void Init(const ByteArray &data);    
    void ParseBody(const ByteArray &data, size_t headerSize);

private:
    int m_connID;
    const HttpConfig &m_config;
    HttpHeader m_header;
    ByteArray m_data;
    std::map<std::string, std::string> m_args;    
    RequestBody m_requestBody;
};

}

#endif // REQUEST_H
