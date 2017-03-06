#include <fstream>
#include <json/json.h>
#include "log.h"
#include "config.h"


using namespace omnitty;


OmniConfig *OmniConfig::m_instance = nullptr;


OmniConfig::OmniConfig()
    : m_logFilePath("/tmp/omnitty.log"), m_sshUserName("root")
{
    m_configFilePath = getenv("HOME") + std::string("/.omnitty/config.json");
}

bool OmniConfig::LoadConfig()
{
    std::fstream ifstream;
    ifstream.open(m_configFilePath);
    if (!ifstream.is_open()) {
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifstream, root, false)) {
        return false;
    }

    // log
    m_logFilePath = root.get("LogFilePath", "/tmp/omnitty.log").asString();
    m_logConfigFilePath = root.get("LogConfigFilePath", "").asString();
    m_logFormat = root.get("LogFormat", "").asString();

    // machine
    m_machineFilePath = root.get("MachineFilePath", "").asString();

    // ssh
    m_sshUserName = root.get("SSHUserName", "root").asString();
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
    root["LogFilePath"] = m_logFilePath;
    root["LogConfigFilePath"] = m_logConfigFilePath;
    root["LogFormat"] = m_logFormat;
    root["MachineFilePath"] = m_machineFilePath;
    root["SSHUserName"] = m_sshUserName;
    root["SSHParam"] = m_sshParam;

    Json::FastWriter writer;
    std::string fileContent = writer.write(root);
    ofstream << fileContent;
    ofstream.close();
    return true;
}


std::string OmniConfig::GetCommand(const std::string &machineName)
{
    (void)machineName;
    return std::string("");
}

