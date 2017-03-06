#pragma once
#include <string>


namespace omnitty {


class OmniConfig
{
protected:
    OmniConfig();

    ~OmniConfig() = default;

public:
    static OmniConfig *GetInstance() {
        if (m_instance == nullptr) {
            m_instance = new OmniConfig();
        }
        return m_instance;
    }

    bool LoadConfig();

    bool SaveConfig();

    std::string GetCommand(const std::string &machineName);

    const std::string &GetLogFilePath() const { return m_logFilePath; }

    const std::string &GetLogConfigFilePath() const { return m_logConfigFilePath; }

    const std::string &GetLogFormat() const { return m_logFormat; }

    const std::string &GetMachineFilePath() const { return m_machineFilePath; }

    const std::string &GetSshUserName() const { return m_sshUserName; }

    const std::string &GetSshParam() const { return m_sshParam; }


private:
    static OmniConfig   *m_instance;
    std::string         m_configFilePath;
    std::string         m_logFilePath;
    std::string         m_logConfigFilePath;
    std::string         m_logFormat;
    std::string         m_machineFilePath;
    std::string         m_sshUserName;
    std::string         m_sshParam;
};


}

