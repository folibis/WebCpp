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
    bool Parse(const ByteArray &data, const ByteArray &contentType);

    ContentType GetContentType() const;

    struct ContentValue
    {
        std::string name;
        std::string contentType;
        std::string fileName;
        ByteArray data;

        std::string GetDataString() const;
        static ContentValue defaultValue;
    };

    const std::vector<ContentValue>& GetValues() const;
    const ContentValue& GetValue(const std::string &name) const;

protected:
    std::map<std::string, std::string> ParseHeaders(const ByteArray &header) const;
    std::map<std::string, std::string> ParseFields(const ByteArray &header) const;
    std::string GetHeader(const std::string &name, const std::map<std::string, std::string> &map) const;

private:
    std::vector<ContentValue> m_values;
    ContentType m_contentType = ContentType::Undefined;
};

}

#endif // REQUEST_BODY_H
