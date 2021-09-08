#include "StringUtil.h"
#include "ICommunication.h"


using namespace WebCpp;


void ICommunication::ParseAddress(const std::string &address)
{
    if(!address.empty())
    {
        auto addr_arr = StringUtil::Split(address, ':');
        if(addr_arr.size() == 2)
        {
            m_address = addr_arr[0];
            int port;
            if(StringUtil::String2int(addr_arr[1], port))
            {
                m_port = port;
            }
        }
    }
}
