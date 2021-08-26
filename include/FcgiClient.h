#ifndef FCGICLIENT_H
#define FCGICLIENT_H

#include <string>
#include <map>
#include "ComminucationUnixClient.h"
#include "Request.h"
#include "HttpConfig.h"


namespace WebCpp
{

class FcgiClient
{
public:
    enum class FcgiParam
    {
        QUERY_STRING,
        REQUEST_METHOD,
        PATH_INFO,
        CONTENT_TYPE,
        CONTENT_LENGTH,
        SCRIPT_FILENAME,
        SCRIPT_NAME,
        REQUEST_URI,
        DOCUMENT_URI,
        DOCUMENT_ROOT,
        SERVER_PROTOCOL,
        GATEWAY_INTERFACE,
        REMOTE_ADDR,
        REMOTE_PORT,
        SERVER_ADDR,
        SERVER_PORT,
        SERVER_NAME,
    };

    FcgiClient(const std::string &address, int version = 1);
    bool Connect();
    void SetParam(FcgiParam param, std::string name);
    bool SendRequest(const Request& request, const HttpConfig& config);
    std::string GetParam(FcgiParam param, const HttpHeader &header, const HttpConfig &config) const;

protected:
#pragma pack(push, 1)

    enum class RequestType
    {
        FCGI_BEGIN_REQUEST     = 1,
        FCGI_ABORT_REQUEST     = 2,
        FCGI_END_REQUEST       = 3,
        FCGI_PARAMS            = 4,
        FCGI_STDIN             = 5,
        FCGI_STDOUT            = 6,
        FCGI_STDERR            = 7,
        FCGI_DATA              = 8,
        FCGI_GET_VALUES        = 9,
        FCGI_GET_VALUES_RESULT = 10,
        FCGI_UNKNOWN_TYPE      = 11,
    };

    enum class RequestRole
    {
        FCGI_RESPONDER         = 1,
        FCGI_AUTHORIZER        = 2,
        FCGI_FILTER            = 3,
    };

    typedef struct {
        uint8_t version;
        uint8_t type;
        uint8_t requestIdB1;
        uint8_t requestIdB0;
        uint8_t contentLengthB1;
        uint8_t contentLengthB0;
        uint8_t paddingLength;
        uint8_t reserved;
    } FCGI_Header;

    typedef struct {
        uint8_t roleB1;
        uint8_t roleB0;
        uint8_t flags;
        uint8_t reserved[5];
    } FCGI_BeginRequestBody;

    typedef struct {
        FCGI_Header header;
        FCGI_BeginRequestBody body;
    } FCGI_BeginRequestRecord;

    typedef struct {
        uint8_t nameLengthB0;
    } FCGI_Name1;

    typedef struct {
        uint8_t nameLengthB3;
        uint8_t nameLengthB2;
        uint8_t nameLengthB1;
        uint8_t nameLengthB0;
    } FCGI_Name2;

    typedef struct {
        uint8_t valueLengthB0;
    } FCGI_Value1;

    typedef struct {
        uint8_t valueLengthB3;
        uint8_t valueLengthB2;
        uint8_t valueLengthB1;
        uint8_t valueLengthB0;
    } FCGI_Value2;
#pragma pack(pop)

    ByteArray BuildBeginRequestPacket() const;
    ByteArray BuildParamPacket(const std::string &name, const std::string &value) const;
    ByteArray BuildParamsPacket(const ByteArray &params) const;
    ByteArray BuildStdinPacket(const ByteArray &stdinData) const;
    void OnDataReady(ByteArray &data);

private:
    std::string m_address;
    int m_fcgiVersion;
    ComminucationUnixClient m_connection;
    static uint16_t RequestID;
    std::map<FcgiParam, std::string> m_fcgiParams;
};

}

#endif // FCGICLIENT_H
