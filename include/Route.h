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

#ifndef ROUTE_H
#define ROUTE_H

#include <functional>
#include <map>
#include <algorithm>
#include "Request.h"
#include "Response.h"
#include "IHttp.h"


namespace WebCpp
{

class Route
{
public:
    Route(const std::string &path, Http::Method method);
    Route(const Route& other) = delete;
    Route& operator=(const Route& other) = delete;
    Route(Route&& other) = default;
    Route& operator=(Route&& other) = default;

    const std::string& GetPath() const;
    bool IsMatch(Request &request);
    std::string ToString() const;

protected:
    bool Parse(const std::string &path);
    struct Token
    {
        enum class Type
        {
            Default = 0,
            Variable,
            Group,
            Any,
        };
        enum class View
        {
            Default = 0,
            Alpha,
            Numeric,
            String,
            Upper,
            Lower,            
        };

        std::string text = "";
        std::vector<std::string> group;
        Type type = Type::Default;
        View view = View::Default;
        bool optional = false;

        size_t GetLength() const
        {
            return text.length();
        }

        void Clear()
        {
            text = "";
            type = Type::Default;
            view = View::Default;
            group.clear();
            group.shrink_to_fit();
        }
        bool IsEmpty()
        {
            return (text.empty() && group.size() == 0);
        }
        void SortGroup()
        {
            std::sort(group.begin(), group.end(), [](const std::string& first, const std::string& second)
            {
                if(first.size() != second.size())
                {
                    return first.size() > second.size();
                }
                return first > second;

            });
        }

        bool IsMatch(const char *ch, size_t length, size_t& pos);
        bool IsString(char ch) const;
        bool IsAlpha(char ch) const;
        bool IsNumeric(char ch) const;
        bool IsLower(char ch) const;
        bool IsUpper(char ch) const;

        static View String2View(const std::string &str);
        bool Compare(const char *ch1, const char *ch2, size_t size);
    };

    bool AddToken(Token &token, const std::string &str);

private:
    enum class State
    {
        Default = 0,
        Variable,
        Optional,
        VariableType,
        OrGroup,
    };

    std::vector<Token> m_tokens;
    Http::Method m_method;
    std::string m_path;
};

}

#endif // ROUTE_H
