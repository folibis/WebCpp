/*
*
* Copyright (c) 2021 ruslan@muhlinin.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#ifndef WEBCPP_REQUEST_BODY_H
#define WEBCPP_REQUEST_BODY_H

#include "common_webcpp.h"
#include <vector>
#include <map>
#include <string>
#include "IErrorable.h"


namespace WebCpp
{

class RequestBody: public IErrorable
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
    ~RequestBody();

    RequestBody(const RequestBody&other) = delete;
    RequestBody& operator=(const RequestBody& other) = delete;
    RequestBody(RequestBody&& other);
    RequestBody& operator=(RequestBody&& other);

    bool Parse(const ByteArray &data, size_t offset, const ByteArray &contentType, bool useTempFile);

    ContentType GetContentType() const;
    void SetContentType(ContentType type);

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
    void SetValue(const std::string &name, const ByteArray &data, const std::string &contentType = "");
    void SetValue(const std::string &name, const std::string &fileName, const std::string &contentType = "");
    std::string GetTempFolder() const;
    ByteArray ToByteArray();
    std::string BuildContentType() const;
    void Clear();

protected:
    std::map<std::string, std::string> ParseHeaders(const ByteArray &header) const;
    std::map<std::string, std::string> ParseFields(const ByteArray &header) const;
    std::string GetHeader(const std::string &name, const std::map<std::string, std::string> &map) const;
    ContentType ParseContentType(const ByteArray &contentType) const;

    bool ParseFormData(const ByteArray &data, size_t offset, const ByteArray &contentType, bool useTempFile);
    bool ParseUrlEncoded(const ByteArray &data, size_t offset, const ByteArray &contentType);
    bool ParseText(const ByteArray &data, size_t offset, const ByteArray &contentType);

    ByteArray GetDataUrlUncoded() const;
    ByteArray GetDataMultipart();
    ByteArray GetDataText() const;
    void CreateBoundary();

private:
    std::vector<ContentValue> m_values;
    ContentType m_contentType = ContentType::Undefined;
    std::string m_tempFolder;
    std::string m_boundary;
};

}

#endif // WEBCPP_REQUEST_BODY_H
