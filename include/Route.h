#ifndef ROUTE_H
#define ROUTE_H

#include "Request.h"

namespace WebCpp
{

class Route
{
public:
    Route();
    bool IsMatch(const Request& request);
};

}

#endif // ROUTE_H
