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

    uint32_t GetListWndWidth() const { return m_listWndWidth; }

    uint32_t GetSummaryWndWidth() const { return m_summaryWndWidth; }

    uint32_t GetTerminalWndWidth() const { return m_terminalWndWidth; }

    const std::string &GetLogFilePath() const { return m_logFilePath; }

    const std::string &GetLogConfigFilePath() const { return m_logConfigFilePath; }

    const std::string &GetLogFormat() const { return m_logFormat; }

    const std::string &GetMachineFilePath() const { return m_machineFilePath; }

    const std::string &GetSshUserName() const { return m_sshUserName; }

    const std::string &GetSshParam() const { return m_sshParam; }

private:
    static OmniConfig   *m_instance;
    uint32_t            m_listWndWidth;
    uint32_t            m_summaryWndWidth;
    uint32_t            m_terminalWndWidth;
    std::string         m_configFilePath;
    std::string         m_logFilePath;
    std::string         m_logConfigFilePath;
    std::string         m_logFormat;
    std::string         m_machineFilePath;
    std::string         m_sshUserName;
    std::string         m_sshUserPassword;
    std::string         m_sshParam;
};


}

