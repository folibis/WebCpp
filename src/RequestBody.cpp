#include "common_webcpp.h"
//#include <fstream>
#include "StringUtil.h"
#include "RequestBody.h"
#include "FileSystem.h"
#include "File.h"

#define WRITE_BIFFER_SIZE 1024


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
    ClearError();
    bool retval = false;

    if(useTempFile)
    {
        m_tempFolder = FileSystem::TempFolder();
        if(FileSystem::CreateFolder(m_tempFolder) == false)
        {
            SetLastError("error creating temporary folder");
            return false;
        }
    }

    auto type = ParseContentType(contentType);

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
        default:
            SetLastError("undefined content type");
            break;

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
                            File file(m_tempFolder + "/" + filename, File::Mode::Write);
                            file.Write(reinterpret_cast<const char*>(data.data() + chunkHeaderPos + 4), range.end - 1 - (chunkHeaderPos + 4));

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
    else
    {
        SetLastError("boundary not defined");
    }

    return retval;
}

bool RequestBody::ParseUrlEncoded(const ByteArray &data, size_t offset, const ByteArray &contentType)
{
    bool retval = true;

    m_contentType = ContentType::UrlEncoded;
    auto end = StringUtil::SearchPositionReverse(data, { CRLF }, offset);
    auto values = StringUtil::Split(data, {'&'}, offset, end);
    if(values.size() > 0)
    {
        for(auto &value: values)
        {
            auto pair = StringUtil::Split(data, {'='}, value.start, value.end);

            std::string name(data.begin() + pair.at(0).start ,data.begin() + pair.at(0).end + 1);
            std::string val = pair.size() > 1 ? std::string(data.begin() + pair.at(1).start ,data.begin() + pair.at(1).end + 1) : "";
            StringUtil::UrlDecode(name);
            StringUtil::UrlDecode(val);
            m_values.push_back(ContentValue {
                                   name,
                                   std::string(contentType.begin(), contentType.end()),
                                   "",
                                   ByteArray(val.begin(), val.end()) });
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

void RequestBody::SetContentType(ContentType type)
{
    m_contentType = type;
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

void RequestBody::SetValue(const std::string &name, const ByteArray &data, const std::string &contentType)
{
    m_values.push_back({ name, contentType, "", data });
}

void RequestBody::SetValue(const std::string &name, const std::string &fileName, const std::string &contentType)
{
    m_values.push_back({ name, contentType, fileName, {} });
}

std::string RequestBody::GetTempFolder() const
{
    return m_tempFolder;
}

ByteArray RequestBody::ToByteArray()
{
    ClearError();
    ByteArray data;

    switch(m_contentType)
    {
        case ContentType::UrlEncoded:
            data = GetDataUrlUncoded();
            break;
        case ContentType::FormData:
            data = GetDataMultipart();
            break;
        case ContentType::Text:
            data = GetDataText();
            break;
        default: break;
    }

    return data;
}

std::string RequestBody::BuildContentType() const
{
    std::string str = "";
    switch(m_contentType)
    {
        case ContentType::UrlEncoded:
            str += "application/x-www-form-urlencoded;";
            break;
        case ContentType::FormData:
            str += "multipart/form-data;";
            break;
        case ContentType::Text:
            str += "text/plain;";
            break;
        default: break;
    }
    str += "boundary=\"" + m_boundary + "\"";

    return str;
}

ByteArray RequestBody::GetDataUrlUncoded() const
{
    std::string data;
    for(const auto &entity: m_values)
    {
        std::string name = entity.name;
        StringUtil::UrlDecode(name);
        std::string value = StringUtil::ByteArray2String(entity.data);
        StringUtil::UrlDecode(value);
        data += (data.empty() ? "" : "&") + name + "=" + value;
    }

    return StringUtil::String2ByteArray(data);
}

ByteArray RequestBody::GetDataMultipart()
{
    ByteArray data;
    CreateBoundary();
    for(const auto &entity: m_values)
    {
        std::string header = "Content-Disposition: form-data; name=" + entity.name;
        if(!entity.fileName.empty())
        {
            header += "; filename=\"" + entity.fileName + "\"";
        }
        header += CR + LF;
        if(!entity.contentType.empty())
        {
            header += "Content-Type: " + entity.contentType + CR + LF;
        }
        header += (CR + LF);

        data.insert(data.end(), m_boundary.begin(), m_boundary.end());
        data.insert(data.end(), {CR, LF});
        data.insert(data.end(), header.begin(), header.end());
        if(!entity.fileName.empty())
        {
            if(FileSystem::IsFileExist(entity.fileName))
            {
                char buffer[WRITE_BIFFER_SIZE];
                File file(entity.fileName, File::Mode::Read);
                size_t bites = 0;
                do
                {
                    bites = file.Read(buffer, WRITE_BIFFER_SIZE);
                    if(bites > 0)
                    {
                        data.insert(data.end(), buffer, buffer + bites);
                    }
                }
                while(bites > 0);
            }
            else
            {
                SetLastError("file not exist");
                break;
            }
        }
        else if(!entity.data.empty())
        {
            data.insert(data.end(), entity.data.begin(), entity.data.end());
        }

        data.insert(data.end(), {CR, LF});
    }
    data.insert(data.end(), m_boundary.begin(), m_boundary.end());
    data.insert(data.end(), { '-','-',CR, LF });

    return data;
}

ByteArray RequestBody::GetDataText() const
{
    if(m_values.size() > 0)
    {
        return m_values.at(0).data;
    }

    return ByteArray();
}

void RequestBody::CreateBoundary()
{
    StringUtil::RandInit();
    std::string b = "----------";
    int len = StringUtil::GetRand(30, 40);
    for(int i = 0;i < len;i ++)
    {
        b += static_cast<char>(StringUtil::GetRand('a','z'));
    }
    m_boundary = b;
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
