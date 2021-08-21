#ifndef IRUNNABE_H
#define IRUNNABE_H

namespace WebCpp
{

class IRunnable
{
public:
    virtual bool Init() { return false; };
    virtual bool Run() = 0;
    virtual bool Close(bool wait = true) = 0;
    virtual bool WaitFor() = 0;

};

}

#endif // IRUNNABE_H
