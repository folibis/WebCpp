#include "File.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>


using namespace WebCpp;

File::File()
{

}

File::File(const std::string &file, Mode mode)
{
    Open(file, mode);
}

File::~File()
{
    Close();
}

bool File::Open(const std::string &file,  Mode mode)
{
    ClearError();

    m_file = file;
    m_mode = mode;
    m_fd = open(m_file.c_str(), Mode2Flag(mode));
    if(m_fd == (-1))
    {
        SetLastError(strerror(errno));
    }

    return IsOpened();
}

bool File::Close()
{
    if(m_fd != (-1))
    {
        close(m_fd);
        m_fd = (-1);
        return true;
    }

    return false;
}

size_t File::Read(char *buffer, size_t size)
{
    return read(m_fd, buffer, size);
}

size_t File::Write(const char *buffer, size_t size)
{
    return write(m_fd, buffer, size);
}

bool File::IsOpened() const
{
    return (m_fd != (-1));
}

int File::Mode2Flag(Mode mode)
{
    if(contains(mode, Mode::Read))
    {
        if(contains(mode, Mode::Write))
        {
            return O_RDWR | O_TRUNC;
        }
        else
        {
            return O_RDONLY;
        }
    }
    else if(contains(mode, Mode::Write))
    {
        return O_WRONLY | O_TRUNC;
    }

    return O_RDONLY;
}
