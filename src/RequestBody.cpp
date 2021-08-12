#include "common.h"
#include <fstream>
#include "RequestBody.h"


using namespace WebCpp;

RequestBody::ContentValue RequestBody::ContentValue::defaultValue = {};

RequestBody::RequestBody()
{

}

bool RequestBody::Parse(const ByteArray &data, const ByteArray &contentType)
{
    bool retval = false;

    size_t pos;

    std::ofstream f("/home/ruslan/html.output", std::ofstream::binary | std::ofstream::trunc);
    f.write(data.data(), data.size());
    f.close();


    if(look_for(contentType, "multipart/form-data", pos))
    {
        m_contentType = ContentType::FormData;
        auto headers = ParseFields(contentType);
        auto boundary = GetHeader("boundary", headers);

        if(!boundary.empty())
        {
            boundary = "--" + boundary + std::string { CRLF };
            auto arr = find_all_entries(data, ByteArray(boundary.begin(), boundary.end()));
            for(auto &p: arr)
            {
            //    std::string s(str.begin() + p.p1, str.begin() + p.p2 + 1);
            //    std::cout << s << std::endl;
            //}

            //auto bodyChunks = split(data, ByteArray(boundary.begin(), boundary.end()));
            //for(auto &chunk: bodyChunks)
            //{


                if(look_for(chunk, ByteArray{ CRLFCRLF }, pos))
                {
                    std::string contentType,name,filename;

                    auto chunkHeader = ByteArray(chunk.begin(), chunk.begin() + pos);
                    auto chunkData = ByteArray(chunk.begin() + pos + 4, chunk.end() - 2); // remove 2*CRLF + trailing CRLF
                    auto chunkHeaders = ParseHeaders(chunkHeader);
                    contentType = GetHeader("Content-Type", chunkHeaders);
                    auto contentDisposition = GetHeader("Content-Disposition", chunkHeaders);
                    if(!contentDisposition.empty())
                    {
                        auto contentDispositionFields = ParseFields(ByteArray(contentDisposition.begin(), contentDisposition.end()));
                        name = GetHeader("name", contentDispositionFields);
                        filename = GetHeader("filename", contentDispositionFields);

                        m_values.push_back(ContentValue {
                                               name,
                                               contentType,
                                               filename,
                                               chunkData });
                    }
                }
            }
        }
    }
    else if(look_for(contentType, "application/x-www-form-urlencoded", pos))
    {
        m_contentType = ContentType::UrlEncoded;
        auto lines = split(data, { CRLF });
        for(auto &line: lines)
        {
            auto pair = split(line, '&');
            if(pair.size() > 0)
            {                
                std::string name(pair[0].begin(), pair[0].end());
                std::string value = pair.size() > 1 ? std::string(pair[1].begin(), pair[1].end()) : "";
                urlDecode(name);
                urlDecode(value);
                m_values.push_back(ContentValue {
                                       name,
                                       std::string(contentType.begin(), contentType.end()),
                                       value,
                                       {} });
            }
        }
        retval = true;
    }
    else if(look_for(contentType, "text/plain", pos))
    {
        m_contentType = ContentType::Text;
        m_values.push_back(ContentValue {
                               "",
                               std::string(contentType.begin(), contentType.end()),
                               "",
                               data });
        retval = true;
    }

    return retval;
}

RequestBody::ContentType RequestBody::GetContentType() const
{
    return m_contentType;
}

const std::vector<RequestBody::ContentValue> &RequestBody::GetValues() const
{
    return m_values;
}

const RequestBody::ContentValue& RequestBody::GetValue(const std::string &name) const
{
    for(auto &value: m_values)
    {
        if(value.name == name)
        {
            return value;
        }
    }

    return RequestBody::ContentValue::defaultValue;
}

std::map<std::string, std::string> RequestBody::ParseHeaders(const ByteArray &header) const
{
    std::map<std::string, std::string> retval;
    auto lines = split(header, { CRLF });
    for(auto &line: lines)
    {
        auto pair = split(line, ':');
        if(pair.size() == 2)
        {
            trim(pair[0]);
            trim(pair[1], { ' ', CRLF });
            retval[std::string(pair[0].begin(), pair[0].end())]
                    = std::string(pair[1].begin(), pair[1].end());
        }
    }

    return retval;
}

std::map<std::string, std::string> RequestBody::ParseFields(const ByteArray &header) const
{
    std::map<std::string, std::string> retval;

    auto headers = split(header, ';');
    std::string boundary = "";
    for(auto &header: headers)
    {
        auto pair = split(header, '=');
        if(pair.size() >= 1)
        {
            trim(pair[0]);
            ByteArray value;
            if(pair.size() >= 2)
            {
                value = pair[1];
                trim(value, { ' ', CRLF, '\"' });
            }
            retval[std::string(pair[0].begin(), pair[0].end())]
                    = std::string(value.begin(), value.end());
        }
    }

    return retval;
}

std::string RequestBody::GetHeader(const std::string &name, const std::map<std::string, std::string> &map) const
{
    auto it = map.find(name);
    if(it != map.end())
    {
        return it->second;
    }

    return "";
}

std::string RequestBody::ContentValue::GetDataString() const
{
    return std::string(data.begin(), data.end());
}
