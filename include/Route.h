#ifndef ROUTE_H
#define ROUTE_H

#include <functional>
#include <map>
#include "Request.h"
#include "Response.h"


namespace WebCpp
{

class Route
{
public:
    using RouteFunc = std::function<bool(const Request&request, Response &response)>;

    Route(const std::string &path, Request::Method method);
    bool SetFunction(const RouteFunc& f);
    const RouteFunc& GetFunction() const;
    bool IsMatch(Request &request);

protected:
    bool Parse(const std::string &path);
    struct Token
    {
        enum class Type
        {
            Default = 0,
            Variable,
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
        }
        bool IsEmpty()
        {
            return text.empty();
        }
        bool IsMatch(const char *ch, size_t length, size_t& pos);
        bool IsString(char ch) const;
        bool IsAlpha(char ch) const;
        bool IsNumeric(char ch) const;
        bool IsLower(char ch) const;
        bool IsUpper(char ch) const;

        static View String2View(const std::string &str);
    };

    bool AddToken(Token &token, const std::string &str);
private:
    enum class State
    {
        Default = 0,
        Variable,
        Optional,
        VariableType,
    };

    RouteFunc m_func;
    std::vector<Token> m_tokens;
    Request::Method m_method;
};

}

#endif // ROUTE_H
