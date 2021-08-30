![example workflow](https://github.com/folibis/WebCpp/actions/workflows/cmake.yml/badge.svg)

# WebCpp

small HTTP/C++11 library, mainly intended for old CPUs/compilers (~420K)

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
- Simple logging
- WebSocket (both ws and wss)
- FastCGI basic support (tested with php-fpm)

Requirements:
- Linux (Windows support comming soon)
- Gcc >= 4.8.3
- CMake
- OpenSSL & OpenSSL headers (optionally)

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
    server.OnGet("/hello", [](const WebCpp::Request& request, WebCpp::Response& response) -> bool
    {
        response.SetHeader("Content-Type","text/html;charset=utf-8");
        response.Write("<div>Hello, world!</div>");
        return true;
    });
    
    server.Run();
}   
```


**POST handling:**

```cpp
server.OnPost("/form", [](const WebCpp::Request& request, WebCpp::Response& response) -> bool
{
    auto &body = request.GetRequestBody();
    auto name = body.GetValue("name");
    auto family = body.GetValue("family");
    
    response.SetHeader("Content-Type","text/html;charset=utf-8");
    response.Write("<p>Hello, " + name + " " + family + "</p>");
    return true;
});
```

**Routing**
```cpp
server.OnGet("/(user|users)/{user:alpha}/[{action:string}/]", [](const WebCpp::Request& request, WebCpp::Response& response) -> bool
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
* | any value, any length | /*.php will work for /index.php, /subfolder/index.php and whatever


**WebSocket handling:**

```cpp
WebCpp::WebSocketServer wsServer;

WebCpp::HttpConfig config;
config.SetWsProtocol("ws");
config.SetWsServerPort(8081);    
    
if(wsServer.Init(config))
{
    wsServer.OnMessage("/ws[/{user}/]", [](const WebCpp::Request &request, WebCpp::ResponseWebSocket &response, const ByteArray &data) -> bool {
        std::string user = request.GetArg("user");
        if(user.empty())
        {
            user = "Unknown";
        }
        response.WriteText("Hello from server, " + user + "! You've sent: " + StringUtil::ByteArray2String(data));
        return true;
    });
}
wsServer.Run();
wsServer.WaitFor();

// now you can connect to the WebSocket server using ws://127.0.0.1:8081/ws or ws://127.0.0.1:8081/ws/john
// (or use included test page: http://127.0.0.1:8080/ws)
```
**FastCGI handling:**
```cpp
    WebCpp::HttpServer httpServer;
    WebCpp::FcgiClient fcgi("/run/php/php7.4-fpm.sock", config);
    if(fcgi.Init())
    {
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::QUERY_STRING, "QUERY_STRING");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REQUEST_METHOD, "REQUEST_METHOD");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SCRIPT_FILENAME, "SCRIPT_FILENAME");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SCRIPT_NAME, "SCRIPT_NAME");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::PATH_INFO, "PATH_INFO");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REQUEST_URI, "REQUEST_URI");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::DOCUMENT_ROOT, "DOCUMENT_ROOT");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_PROTOCOL, "SERVER_PROTOCOL");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::GATEWAY_INTERFACE, "GATEWAY_INTERFACE");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REMOTE_ADDR, "REMOTE_ADDR");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::REMOTE_PORT, "REMOTE_PORT");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_ADDR, "SERVER_ADDR");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_PORT, "SERVER_PORT");
        fcgi.SetParam(WebCpp::FcgiClient::FcgiParam::SERVER_NAME, "SERVER_NAME");

        fcgi.SetOnResponseCallback([](WebCpp::Response &response) {
            httpServer.SendResponse(response);
        });
    }
    
    if(httpServer.Init(config))
    {
        httpServer.OnGet("/index.php", [&](const WebCpp::Request &request, WebCpp::Response &response) -> bool
        {
            bool retval = false;

            retval = fcgi.SendRequest(request);
            if(retval == false)
            {
                response.SendNotFound();
            }
            else
            {
                response.SetShouldSend(false);
            }
            return true;
        });
    }
```
