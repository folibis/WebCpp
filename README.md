![example workflow](https://github.com/folibis/WebCpp/actions/workflows/cmake/badge.svg)

# WebCpp

simple HTTP/HTTPS C++11 library, mainly intended for old CPUs/compilers

*Note: the code in the midst of development, use it at your own risk*

Currently supported:
- HTTP
- HTTPS (using OpenSSL)
- GET
- POST
- Routing (no std::regex using) 
- Pre/post routing handlers
- Keep-alive
- Simple settings

### Usage: ###

**GET handling:**

```cpp
WebCpp::HttpServer server;

WebCpp::HttpConfig config;
config.SetRoot(PUBLIC_DIR);
config.SetProtocol("HTTP");
config.SetServerPort(8080);

if(server.Init(config))
{
    server.Get("/hello", [](const WebCpp::Request &, WebCpp::Response &response) -> bool
    {
        response.SetHeader("Content-Type","text/html;charset=utf-8");
        response.Write("<div>Hello, world!</div>");
    });
    
    server.Run();
}   
```
**POST handling:**

```cpp
server.Post("/form", [](const WebCpp::Request &, WebCpp::Response &response) -> bool
{
    auto &body = request.GetRequestBody();
    auto name = body.GetValue("name");
    auto family = body.GetValue("family");
    
    response.SetHeader("Content-Type","text/html;charset=utf-8");
    response.Write("<p>Hello, " + name + " " + family + "</p>");
});
```

**Routing**
```cpp
server.Get("/(user|users)/{user:alpha}/[{action:string}/]", [](const WebCpp::Request &request, WebCpp::Response &response) -> bool
{
    std::string user = request.GetArg("user");
    if(user.empty())
    {
        user = "Unknown";
    }
    
    std::string action = request.GetArg("action");
    if(action.empty())
    {
        action = "Hello!";
    }

    response.SetHeader("Content-Type","text/html;charset=utf-8");
    response.Write(std::string("<h2>") + user + ", " + action + "</h2>");
    return true;
});
// can be tested using URL like /user/john/hello 
// or
// /users/children/clap%20your%20hands
```
**Routing placeholders**
Placeholder | Notes | Example
------------ | ------------- | -------------
(value1\|value2) | miltiple values | /(user\|users) will work for /user, /users
{variable} | capturing variable | /user/{name} will work for /user/john and the variable can be retrived in a handler using `request.GetArg("name")`
{variable:xxx} | variable type | xxx is one of [alpha, numeric, string, upper, lower], that allow to narrow down a variable type
[optional] | optional value | /user/[num] will work for /user, /user/2




