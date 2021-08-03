#include "RequestBody.h"

using namespace WebCpp;

RequestBody::RequestBody()
{

}

bool RequestBody::Parse(const ByteArray &data, const std::string &contentType)
{
    bool retval = false;

    size_t pos;
    if(look_for(contentType, "multipart/form-data", pos))
    {
        m_contentType = ContentType::FormData;
        auto headers = split(contentType, ';');
        std::string boundary = "";
        for(auto &header: headers)
        {
            auto pair = split(header, '=');
            if(pair.size() == 2)
            {
                trim(pair[0]);
                trim(pair[1], { ' ', CRLF, '\"' });
                if(pair[0] == "boundary")
                {
                    boundary = pair[1];
                    break;
                }
            }
        }

        if(!boundary.empty())
        {
            auto bodyChunks = split(data, ByteArray(boundary.begin(), boundary.end()));
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
                m_values.push_back(ContentValue {name, contentType, value, {} });
            }
        }
        retval = true;
    }
    else if(look_for(contentType, "text/plain", pos))
    {
        m_contentType = ContentType::Text;
        m_values.push_back(ContentValue {"", contentType, "", data });
        retval = true;
    }

    return retval;
}

RequestBody::ContentType RequestBody::GetContentType() const
{
    return m_contentType;
}
