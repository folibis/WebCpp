#include <cstring>
#include <iostream>
#include <iomanip>
#include "FcgiClient.h"


using namespace WebCpp;

uint16_t FcgiClient::RequestID = 1;

FcgiClient::FcgiClient(const std::string &address, int version):
    m_connection(address)
{
    m_address = address;
    m_fcgiVersion = version;
}

bool FcgiClient::Connect()
{
    bool retval = false;

    if(m_connection.Init())
    {
        if(m_connection.Connect())
        {
            auto f = std::bind(&FcgiClient::OnDataReady, this, std::placeholders::_1);
            m_connection.SetDataReadyCallback(f);
            m_connection.Run();
            retval = true;
        }
    }

    return retval;
}

void FcgiClient::SetParam(FcgiClient::FcgiParam param, std::string name)
{
    m_fcgiParams[param] = name;
}

bool FcgiClient::SendRequest(const Request &request, const HttpConfig &config)
{
    //bool retval = false;

    ByteArray requestDataData;

    ByteArray beginPacket = BuildBeginRequestPacket();
    requestDataData.insert(requestDataData.end(), beginPacket.begin(), beginPacket.end());

    ByteArray paramsData;
    const auto &header = request.GetHeader();
    for(auto &pair: m_fcgiParams)
    {
        FcgiClient::FcgiParam param = pair.first;
        std::string name = pair.second;
        std::string value = GetParam(param, header, config);
        ByteArray paramBytes = BuildParamPacket(name, value);
        paramsData.insert(paramsData.end(), paramBytes.begin(), paramBytes.end());
    }

    paramsData = BuildParamsPacket(paramsData);
    ByteArray finalParam = BuildParamsPacket(ByteArray());
    paramsData.insert(paramsData.end(), finalParam.begin(), finalParam.end());

    requestDataData.insert(requestDataData.end(), paramsData.begin(), paramsData.end());

    ByteArray stdinPacket = BuildStdinPacket(ByteArray());
    requestDataData.insert(requestDataData.end(), stdinPacket.begin(), stdinPacket.end());    

    return m_connection.Write(requestDataData);
}

std::string FcgiClient::GetParam(FcgiClient::FcgiParam param, const HttpHeader& header, const HttpConfig &config) const
{
    switch(param)
    {
        case FcgiParam::QUERY_STRING:
            return header.GetQuery();
        case FcgiParam::REQUEST_METHOD:
            return HttpHeader::Method2String(header.GetMethod());
            break;
        case FcgiParam::PATH_INFO:
            return "";
            break;
        case FcgiParam::CONTENT_TYPE:
            return header.GetHeader(HttpHeader::HeaderType::ContentType);
            break;
        case FcgiParam::CONTENT_LENGTH:
            return header.GetHeader(HttpHeader::HeaderType::ContentLength);
            break;
        case FcgiParam::SCRIPT_FILENAME:
            return config.RootFolder() + header.GetPath();
            break;
        case FcgiParam::SCRIPT_NAME:
            return header.GetPath();
        case FcgiParam::REQUEST_URI:
            return header.GetUri();
        case FcgiParam::DOCUMENT_URI:
            return "";
            break;
        case FcgiParam::DOCUMENT_ROOT:
            return config.RootFolder();
            break;
        case FcgiParam::SERVER_PROTOCOL:
            return header.GetVersion();
        case FcgiParam::GATEWAY_INTERFACE:
            return "CGI/1.1";
        case FcgiParam::REMOTE_ADDR:
            return header.GetRemoteAddress();
        case FcgiParam::REMOTE_PORT:
            return std::to_string(header.GetRemotePort());
            break;
        case FcgiParam::SERVER_ADDR:
            config.GetHttpServerAddress();
        case FcgiParam::SERVER_PORT:
            return std::to_string(config.GetHttpServerPort());
        case FcgiParam::SERVER_NAME:
            return config.GetServerName();
    }

    return "";
}

ByteArray FcgiClient::BuildBeginRequestPacket() const
{
    uint16_t ID = FcgiClient::RequestID;
    uint16_t length = sizeof(FCGI_BeginRequestBody);

    FCGI_BeginRequestRecord record = {};
    record.header.version = static_cast<uint8_t>(m_fcgiVersion);
    record.header.type = static_cast<uint8_t>(RequestType::FCGI_BEGIN_REQUEST);
    record.header.requestIdB0 = static_cast<uint8_t>(ID & 0xFF);
    record.header.requestIdB1 = static_cast<uint8_t>(ID >> 8 & 0xFF);
    record.header.contentLengthB0 = static_cast<uint8_t>(length & 0xFF);
    record.header.contentLengthB1 = static_cast<uint8_t>(length >> 8 & 0xFF);

    uint16_t role = static_cast<uint16_t>(RequestRole::FCGI_RESPONDER);
    record.body.roleB0 = static_cast<uint8_t>(role & 0xFF);
    record.body.roleB1 = static_cast<uint8_t>(role >> 8 & 0xFF);

    ByteArray data(sizeof(record));
    std::memcpy(data.data(), &record, sizeof(record));
    return data;
}

ByteArray FcgiClient::BuildParamPacket(const std::string &name, const std::string &value) const
{
    ByteArray data;
    size_t nlen = name.size();
    size_t vlen = value.size();
    char *ptr;
    size_t recordSize;

    if(nlen < 128)
    {
        FCGI_Name1 nameRecord;
        nameRecord.nameLengthB0 = nlen;
        ptr = reinterpret_cast<char *>(&nameRecord);
        recordSize = sizeof(nameRecord);
        data.insert(data.end(), ptr, ptr + recordSize);
    }
    else
    {
        FCGI_Name2 nameRecord;
        nameRecord.nameLengthB0 = nlen & 0xFF;
        nameRecord.nameLengthB1 = nlen >> 8 & 0xFF;
        nameRecord.nameLengthB2 = nlen >> 16 & 0xFF;
        nameRecord.nameLengthB3 = nlen >> 24 | 0x80;
        ptr = reinterpret_cast<char *>(&nameRecord);
        recordSize = sizeof(nameRecord);
        data.insert(data.end(), ptr, ptr + recordSize);
    }    

    if(vlen < 128)
    {
        FCGI_Value1 valueRecord;
        valueRecord.valueLengthB0 = vlen;
        ptr = reinterpret_cast<char *>(&valueRecord);
        recordSize = sizeof(valueRecord);
        data.insert(data.end(), ptr, ptr + recordSize);
    }
    else
    {
        FCGI_Value2 valueRecord;
        valueRecord.valueLengthB0 = vlen & 0xFF;
        valueRecord.valueLengthB1 = vlen >> 8 & 0xFF;
        valueRecord.valueLengthB2 = vlen >> 16 & 0xFF;
        valueRecord.valueLengthB3 = vlen >> 24 | 0x80;
        ptr = reinterpret_cast<char *>(&valueRecord);
        recordSize = sizeof(valueRecord);
        data.insert(data.end(), ptr, ptr + recordSize);
    }

    data.insert(data.end(), name.begin(), name.end());
    data.insert(data.end(), value.begin(), value.end());

    return data;
}

ByteArray FcgiClient::BuildParamsPacket(const ByteArray &params) const
{
    ByteArray data;

    FCGI_Header header;
    size_t dataSize = params.size();
    uint16_t ID = FcgiClient::RequestID;
    header.version = static_cast<uint8_t>(m_fcgiVersion);
    header.type = static_cast<uint8_t>(RequestType::FCGI_PARAMS);
    header.requestIdB0 = static_cast<uint8_t>(ID & 0xFF);
    header.requestIdB1 = static_cast<uint8_t>(ID >> 8 & 0xFF);
    header.contentLengthB0 = dataSize & 0xFF;
    header.contentLengthB1 = dataSize >> 8 & 0xFF;

    char *ptr = reinterpret_cast<char *>(&header);

    data.insert(data.begin(), ptr, ptr + sizeof(header));

    if(params.size() > 0)
    {
        data.insert(data.end(), params.begin(), params.end());
    }

    return data;
}

ByteArray FcgiClient::BuildStdinPacket(const ByteArray &stdinData) const
{
    FCGI_Header header;
    size_t dataSize = stdinData.size();
    uint16_t ID = FcgiClient::RequestID;
    header.version = static_cast<uint8_t>(m_fcgiVersion);
    header.type = static_cast<uint8_t>(RequestType::FCGI_STDIN);
    header.requestIdB0 = static_cast<uint8_t>(ID & 0xFF);
    header.requestIdB1 = static_cast<uint8_t>(ID >> 8 & 0xFF);
    header.contentLengthB0 = dataSize & 0xFF;
    header.contentLengthB1 = dataSize >> 8 & 0xFF;

    ByteArray data;
    char *ptr = reinterpret_cast<char *>(&header);
    data.insert(data.begin(), ptr, ptr + sizeof(header));
    if(dataSize > 0)
    {
        data.insert(data.begin(), stdinData.begin(), stdinData.end());
    }

    return data;
}

void FcgiClient::OnDataReady(ByteArray &data)
{
    std::cout << "data ready: " << StringUtil::ByteArray2String(data) << std::endl;
}




