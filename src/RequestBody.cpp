#include "common.h"
#include <fstream>
#include "StringUtil.h"
#include "RequestBody.h"
#include  "FileSystem.h"


using namespace WebCpp;

RequestBody::ContentValue RequestBody::ContentValue::defaultValue = {};

RequestBody::RequestBody()
{

}

RequestBody::~RequestBody()
{
    if(!m_tempFolder.empty() && FileSystem::IsFileExist(m_tempFolder))
    {
        FileSystem::DeleteFolder(m_tempFolder);
    }
}

RequestBody::RequestBody(RequestBody &&other)
{
    m_values = std::move(other.m_values);
    m_contentType = other.m_contentType;
    m_tempFolder = other.m_tempFolder;

    other.m_contentType = ContentType::Undefined;
    other.m_tempFolder = "";
}

RequestBody &RequestBody::operator=(RequestBody &&other)
{
    m_values = std::move(other.m_values);
    m_contentType = other.m_contentType;
    m_tempFolder = other.m_tempFolder;

    other.m_values.clear();
    other.m_values.shrink_to_fit();
    other.m_contentType = ContentType::Undefined;
    other.m_tempFolder = "";

    return *this;
}

bool RequestBody::Parse(const ByteArray &data, size_t offset, const ByteArray &contentType, bool useTempFile)
{
    bool retval = false;    

    if(useTempFile)
    {
        m_tempFolder = FileSystem::TempFolder();
        FileSystem::CreateFolder(m_tempFolder);
    }

    auto type = ParseContentType(contentType);

    //std::ofstream f("/home/ruslan/html.output", std::ofstream::binary | std::ofstream::trunc);
    //f.write(data.data(), data.size());
    //f.close();

    switch(type)
    {
        case ContentType::FormData:
            retval = ParseFormData(data, offset, contentType, useTempFile);
            break;
        case  ContentType::UrlEncoded:
            retval = ParseUrlEncoded(data, offset, contentType);
            break;
        case ContentType::Text:
            retval = ParseText(data, offset, contentType);
        default: break;

    }

    return retval;
}

bool RequestBody::ParseFormData(const ByteArray &data, size_t offset, const ByteArray &contentType, bool useTempFile)
{
    bool retval = true;

    m_contentType = ContentType::FormData;
    auto headers = ParseFields(contentType);
    auto boundary = GetHeader("boundary", headers);

    if(!boundary.empty())
    {
        std::string commonBoundary =  "--" + boundary + std::string { CRLF };
        std::string finalBoundary = "--" + boundary + "--";
        size_t end = StringUtil::SearchPositionReverse(data, ByteArray(finalBoundary.begin(), finalBoundary.end()));
        if(end != SIZE_MAX)
        {
            end = end - 1;
        }

        auto ranges = StringUtil::SplitReverse(data, ByteArray(commonBoundary.begin(), commonBoundary.end()), offset, end);
        for(auto &range: ranges)
        {
            if(range.end > range.start)
            {
                auto chunkHeaderPos = StringUtil::SearchPosition(data, ByteArray{ CRLFCRLF }, range.start, range.end);
                if(chunkHeaderPos != SIZE_MAX)
                {
                    auto chunkHeader = ByteArray(data.begin() + range.start, data.begin() + chunkHeaderPos);
                    auto chunkHeaders = ParseHeaders(chunkHeader);
                    std::string name,filename;
                    std::string contentType = GetHeader("Content-Type", chunkHeaders);
                    auto contentDisposition = GetHeader("Content-Disposition", chunkHeaders);
                    if(!contentDisposition.empty())
                    {
                        auto contentDispositionFields = ParseFields(ByteArray(contentDisposition.begin(), contentDisposition.end()));
                        name = GetHeader("name", contentDispositionFields);
                        filename = GetHeader("filename", contentDispositionFields);
                        StringUtil::Trim(filename,"\" ");

                        if(useTempFile && !filename.empty())
                        {
                            std::ofstream f(m_tempFolder + "/" + filename, std::ofstream::binary | std::ofstream::trunc);
                            f.write(data.data() + chunkHeaderPos + 4, range.end - 1 - (chunkHeaderPos + 4));
                            f.close();
                            m_values.push_back(ContentValue {
                                                   name,
                                                   contentType,
                                                   filename,
                                                   {} });
                        }
                        else
                        {
                            m_values.push_back(ContentValue {
                                                   name,
                                                   contentType,
                                                   filename,
                                                   ByteArray(data.begin() + chunkHeaderPos + 4, data.begin() + range.end - 1) });
                        }
                    }
                }
            }
        }
    }

    return retval;
}

bool RequestBody::ParseUrlEncoded(const ByteArray &data, size_t offset, const ByteArray &contentType)
{
    bool retval = true;

    m_contentType = ContentType::UrlEncoded;
    auto ranges = StringUtil::Split(data, { CRLF }, offset);
    for(auto &range: ranges)
    {
        auto pair = StringUtil::Split(data, {'&'}, range.start, range.end);
        if(pair.size() > 0)
        {
            std::string name(data.begin() + pair.at(0).start ,data.begin() + pair.at(0).end);
            std::string value = pair.size() > 1 ? std::string(data.begin() + pair.at(1).start ,data.begin() + pair.at(1).end) : "";
            StringUtil::UrlDecode(name);
            StringUtil::UrlDecode(value);
            m_values.push_back(ContentValue {
                                   name,
                                   std::string(contentType.begin(), contentType.end()),
                                   value,
                                   {} });
        }
    }
    retval = true;
    return retval;
}

bool RequestBody::ParseText(const ByteArray &data, size_t offset, const ByteArray &contentType)
{
    bool retval = true;

    m_contentType = ContentType::Text;
    m_values.push_back(ContentValue {
                           "",
                           std::string(contentType.begin(), contentType.end()),
                           "",
                           ByteArray(data.begin() + offset, data.end())
                       });
    retval = true;
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

std::string RequestBody::GetTempFolder() const
{
    return m_tempFolder;
}

std::map<std::string, std::string> RequestBody::ParseHeaders(const ByteArray &header) const
{
    std::map<std::string, std::string> retval;
    auto ranges = StringUtil::Split(header, { CRLF });
    for(auto &range: ranges)
    {
        auto pair = StringUtil::Split(header, {':'}, range.start, range.end);
        if(pair.size() == 2)
        {
            std::string name(header.begin() + pair.at(0).start, header.begin() + pair.at(0).end + 1);
            std::string value(header.begin() + pair.at(1).start, header.begin() + pair.at(1).end + 1);
            StringUtil::Trim(name);
            StringUtil::Trim(value, { ' ', CRLF });
            retval[name] = value;
        }
    }

    return retval;
}

std::map<std::string, std::string> RequestBody::ParseFields(const ByteArray &header) const
{
    std::map<std::string, std::string> retval;

    auto ranges = StringUtil::Split(header, {';'});
    std::string boundary = "";
    for(auto &range: ranges)
    {
        auto pair = StringUtil::Split(header, {'='}, range.start, range.end);
        if(pair.size() >= 1)
        {
            std::string name(header.begin() + pair.at(0).start, header.begin() + pair.at(0).end + 1);
            StringUtil::Trim(name);
            std::string value;
            if(pair.size() >= 2)
            {
                value = std::string(header.begin() + pair.at(1).start, header.begin() + pair.at(1).end + 1);
                StringUtil::Trim(value, { ' ', CRLF, '\"' });
            }
            retval[name] = value;
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

RequestBody::ContentType RequestBody::ParseContentType(const ByteArray &contentType) const
{
    if(StringUtil::SearchPosition(contentType, StringUtil::String2ByteArray("multipart/form-data")) != SIZE_MAX)
    {
        return ContentType::FormData;
    }
    else if(StringUtil::SearchPosition(contentType, StringUtil::String2ByteArray("application/x-www-form-urlencoded")) != SIZE_MAX)
    {
        return ContentType::UrlEncoded;
    }
    else if(StringUtil::SearchPosition(contentType, StringUtil::String2ByteArray("text/plain")) != SIZE_MAX)
    {
        return ContentType::Text;
    }

    return ContentType::Undefined;
}

std::string RequestBody::ContentValue::GetDataString() const
{
    return std::string(data.begin(), data.end());
}
