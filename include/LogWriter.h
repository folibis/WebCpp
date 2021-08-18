#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <string>
#include <fstream>

#define LOG(S,T) LogWriter::Instance().Write(S,T)


class LogWriter
{
public:
    enum class LogType
    {
        Info = 0,
        Error = 1,
        Access = 2,        
    };

    ~LogWriter();
    static LogWriter& Instance();
    void Write(const std::string &text, LogType type = LogType::Info);

protected:
    LogWriter();    

    bool Open();
    bool Close();
    static std::string LogType2String(LogType type);

private:
    bool m_opened = false;
    std::ofstream m_streams[3];
};

#endif // LOGWRITER_H
