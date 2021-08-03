#ifndef REQUEST_BODY_H
#define REQUEST_BODY_H

#include "common.h"
#include <vector>
#include <map>
#include <string>


namespace WebCpp
{

class RequestBody
{
public:

    enum class ContentType
    {
        Undefined,
        UrlEncoded,
        FormData,
        Text,
    };

    RequestBody();
    bool Parse(const ByteArray &data, const std::string &contentType);

    ContentType GetContentType() const;

    struct ContentValue
    {
        std::string name;
        std::string contentType;
        std::string fileName;
        ByteArray data;
    };

    const std::vector<ContentValue>& GetValues() const;

private:
    std::vector<ContentValue> m_values;
    ContentType m_contentType = ContentType::Undefined;
};

}

#endif // REQUEST_BODY_H
