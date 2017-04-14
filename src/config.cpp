#include <fstream>
#ifdef __APPLE__
#include <json/json.h>
#else
#include <jsoncpp/json/json.h>
#endif
#include "log.h"
#include "config.h"


using namespace omnitty;


OmniConfig *OmniConfig::m_instance      = nullptr;
static const int TERM_WND_MIN_WIDTH     = 80;


OmniConfig::OmniConfig()
    : m_listWndWidth(15), m_summaryWndWidth(15), m_terminalWndWidth(80),
      m_logFilePath("/tmp/omnitty.log"), m_logFormat("%d{%y-%m-%d %H:%M:%S} %p %l %m%n"),
      m_sshUserName("root")
{
    m_configFilePath = getenv("HOME") + std::string("/.omnitty/config.json");
}

bool OmniConfig::LoadConfig()
{
    std::fstream ifstream;
    ifstream.open(m_configFilePath, std::ios::in);
    if (!ifstream.is_open()) {
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifstream, root, false)) {
        return false;
    }

    // window size
    m_listWndWidth = root.get("ListWindowWidth", 0).asUInt();
    m_summaryWndWidth = root.get("SummaryWindowWidth", 0).asUInt();
    m_terminalWndWidth = root.get("TerminalWindowWidth", 0).asUInt();

    if (m_terminalWndWidth < TERM_WND_MIN_WIDTH) {
        m_terminalWndWidth = TERM_WND_MIN_WIDTH;
    }

    // log
    m_logFilePath = root.get("LogFilePath", "/tmp/omnitty.log").asString();
    m_logConfigFilePath = root.get("LogConfigFilePath", "").asString();
    m_logFormat = root.get("LogFormat", "%d{%y-%m-%d %H:%M:%S} %p [%l]%n %m").asString();

    // machine
    m_machineFilePath = root.get("MachineFilePath", "").asString();

    // ssh
    m_sshUserName = root.get("SSHUserName", "root").asString();
    m_sshUserPassword = root.get("SSHUserPassword", "").asString();
    m_sshParam = root.get("SSHParam", "").asString();

    ifstream.close();
    return true;
}


bool OmniConfig::SaveConfig()
{
    std::fstream ofstream;
    ofstream.open(m_configFilePath, std::ios::out|std::ios::trunc);
    if (!ofstream.is_open()) {
        return false;
    }

    Json::Value root;
    root["ListWindowWidth"] = m_listWndWidth;
    root["SummaryWindowWidth"] = m_summaryWndWidth;
    root["TerminalWindowWidth"] = m_terminalWndWidth;

    root["LogFilePath"] = m_logFilePath;
    root["LogConfigFilePath"] = m_logConfigFilePath;
    root["LogFormat"] = m_logFormat;

    root["MachineFilePath"] = m_machineFilePath;

    root["SSHUserName"] = m_sshUserName;
    root["SSHUserPassword"] = m_sshUserPassword;
    root["SSHParam"] = m_sshParam;

    Json::FastWriter writer;
    std::string fileContent = writer.write(root);
    ofstream << fileContent;
    ofstream.close();
    return true;
}


std::string OmniConfig::GetCommand(const std::string &machineName)
{
    std::string command("/usr/bin/ssh ");
    command += m_sshParam;
    command = command + " " + m_sshUserName + "@" + machineName;
    if (!m_sshUserPassword.empty()) {
        command = "/usr/local/bin/sshpass -p " + m_sshUserPassword + " " + command;
    }
    return command;
}

