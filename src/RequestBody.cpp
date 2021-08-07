#include "RequestBody.h"

using namespace WebCpp;

RequestBody::RequestBody()
{

}

bool RequestBody::Parse(const ByteArray &data, const ByteArray &contentType)
{
    bool retval = false;

    size_t pos;

    if(look_for(contentType, "multipart/form-data", pos))
    {
        m_contentType = ContentType::FormData;
        auto headers = GetHeaders(contentType);
        auto boundary = GetHeader("boundary", headers);

        if(!boundary.empty())
        {
            auto bodyChunks = split(data, ByteArray(boundary.begin(), boundary.end()));
            for(auto &chunk: bodyChunks)
            {
                if(look_for(chunk, ByteArray{ CRLFCRLF }, pos))
                {
                    auto chunkHeader = ByteArray(chunk.begin(), chunk.begin() + pos);
                    auto chunkData = ByteArray(chunk.begin() + pos + 4, chunk.end());
                    auto chunkHeaders = GetHeaders(chunkHeader);
                    m_values.push_back(ContentValue {
                                           GetHeader("name", chunkHeaders),
                                           GetHeader("content-type", chunkHeaders),
                                           GetHeader("filename", chunkHeaders),
                                           chunkData });
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
                m_values.push_back(ContentValue {
                                       name,
                                       std::string(contentType.begin(),
                                       contentType.end()),
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
                               std::string(contentType.begin(),
                               contentType.end()),
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

std::map<std::string, std::string> RequestBody::GetHeaders(const ByteArray &header) const
{
    std::map<std::string, std::string> retval;

    auto lines = split(header, { CRLF });
    for(auto &line: lines)
    {
        auto headers = split(line, ';');
        std::string boundary = "";
        for(auto &header: headers)
        {
            auto pair = split(header, '=');
            if(pair.size() == 2)
            {
                trim(pair[0]);
                trim(pair[1], { ' ', CRLF, '\"' });
                retval[std::string(pair[0].begin(), pair[0].end())]
                        = std::string(pair[1].begin(), pair[1].end());
            }
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
