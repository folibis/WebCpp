#include "Route.h"

#define FUNC_PTR(object,ptrToMember)  ((object)->*(ptrToMember))

using namespace WebCpp;

Route::Route(const std::string &path, Request::Method method)
{
    m_method = method;
    Parse(path);
}

bool Route::SetFunction(const Route::RouteFunc &f)
{
    m_func = f;
    return true;
}

bool Route::IsMatch(Request &request)
{
    if(request.GetMethod() != m_method)
    {
        return false;
    }

    const std::string path = request.GetPath();
    const char *ch = path.data();
    size_t length = path.length();

    size_t pos = 0;
    size_t offset = 0;

    for(auto &token: m_tokens)
    {
        if(token.IsMatch(ch + pos, length - pos, offset))
        {
            if(token.type == Token::Type::Variable)
            {
                request.SetArg(token.text, std::string(ch + pos, offset));
            }
            pos += offset;
        }
        else
        {
            if(token.optional == false)
            {
                return false;
            }
        }
    }

    if(pos < length)
    {
        return false;
    }

    return true;
}

bool Route::Parse(const std::string &path)
{
    const char *ch = path.data();
    std::string str = "";
    State state = State::Default;
    Token current;

    for(size_t i = 0;i < path.size();i ++)
    {
        switch(ch[i])
        {
            case '[':
                AddToken(current, str);
                str = "";
                current.optional = true;
                state = State::Optional;
                break;
            case ']':
                AddToken(current, str);
                str = "";
                current.optional = false;
                state = State::Default;
                break;
            case '}':
                if(state == State::VariableType)
                {
                    current.view = Token::String2View(str);
                }
                AddToken(current, "");
                str = "";
                state = State::Default;
                break;
            case'{':
                switch(state)
                {
                    case State::Default:
                    case State::Optional:
                        AddToken(current, str);
                        str = "";
                        break;
                    default: break;
                }

                state = State::Variable;
                current.type = Token::Type::Variable;
                break;
            case ':':
                if(state == State::Variable)
                {
                    state = State::VariableType;
                    current.text = str;
                    str = "";
                }
                else
                {
                    str += ch[i];
                }
                break;
            default:
                str += ch[i];
        }
    }

    AddToken(current, str);

    return true;
}

bool Route::AddToken(Token &token, const std::string &str)
{
    if(!str.empty() || !token.IsEmpty())
    {
        if(!str.empty())
        {
            token.text = str;
        }
        m_tokens.push_back(token);
        token.Clear();

        return true;
    }

    return false;
}

bool Route::Token::IsMatch(const char *ch, size_t length, size_t &pos)
{
    bool retval = false;

    switch(type)
    {
        case Type::Default:
            if(text.length() > length)
            {
                break;
            }
            if(compare(ch, text.data(), text.length()))
            {
                pos = text.length();
                retval = true;
            }
            break;
        case Type::Variable:

            bool(Route::Token::*fptr)(char) const = &Route::Token::IsString;

            switch(view)
            {
                case View::Alpha:
                    fptr = &Route::Token::IsAlpha;
                    break;
                case View::Numeric:
                    fptr = &Route::Token::IsNumeric;
                    break;
                case View::String:
                    fptr = &Route::Token::IsString;
                    break;
                case View::Upper:
                    fptr = &Route::Token::IsUpper;
                    break;
                case View::Lower:
                    fptr = &Route::Token::IsLower;
                    break;
                default: break;
            }

            auto f = std::mem_fn(fptr);
            size_t i = 0;
            for(i = 0;i < length;i ++)
            {
                if(f(this, ch[i]) == false)
                {                    
                    break;
                }
            }

            retval = (i > 0);
            if(retval)
            {
                pos = i;
            }

            break;
    }

    return retval;
}

bool Route::Token::IsString(char ch) const
{
    return (IsAlpha(ch) || IsNumeric(ch));
}

bool Route::Token::IsAlpha(char ch) const
{
    return (IsLower(ch) || IsUpper(ch));
}

bool Route::Token::IsNumeric(char ch) const
{
    return (ch >= '0' && ch <= '9');
}

bool Route::Token::IsLower(char ch) const
{
    return (ch >= 'a' && ch <= 'z');
}

bool Route::Token::IsUpper(char ch) const
{
    return (ch >= 'A' && ch <= 'Z');
}

Route::Token::View Route::Token::String2View(const std::string &str)
{
    std::string s = str;
    toLower(s);
    switch(_(s.c_str()))
    {
        case _("alpha"): return Route::Token::View::Alpha;
        case _("numeric"): return Route::Token::View::Numeric;
        case _("string"): return Route::Token::View::String;
        case _("upper"): return Route::Token::View::Upper;
        case _("lower"): return Route::Token::View::Lower;
        default: break;
    }

    return Route::Token::View::Default;
}
