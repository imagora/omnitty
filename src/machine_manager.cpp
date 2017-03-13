#include <fstream>
#include <algorithm>
#include "log.h"
#include "config.h"
#include "curutil.h"
#include "machine.h"
#include "machine_manager.h"


#define MACHINE_MAX 256


using namespace omnitty;


OmniMachineManager::OmniMachineManager()
    : m_isMulticast(false), m_selectedMachine(0), m_scrollPos(0),
      m_virtualTerminalRows(0), m_virtualTerminalCols(0)
{
}


OmniMachineManager::~OmniMachineManager()
{

}

bool OmniMachineManager::LoadMachines(const std::string &fileName)
{
    std::ifstream fileStream(fileName);
    if (!fileStream.is_open()) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "cannot open machine list file: %s", fileName.c_str());
        return false;
    }

    std::string line;
    std::string group("default");
    while (std::getline(fileStream, line)) {
        if (line[0] == '[') {
            if (line.size() > 2) {
                group = std::string(line, 1, line.size() - 2);
            }
            continue;
        }
        m_machineGroups[group].insert(line);
    }
    fileStream.close();
    fileStream.clear();

    std::for_each(m_machineGroups.begin(), m_machineGroups.end(), [&](const std::pair<MachineGroup, std::set<MachineIp>> &groups){
        LOG4CPLUS_INFO_FMT(omnitty::LOGGER_NAME, "load [%s] machine count: %lu", groups.first.c_str(), groups.second.size());
    });
    return true;
}

void OmniMachineManager::AddMachine(const std::string &machineName)
{
    if (machineName.empty()) return;
    if (m_machines.size() >= MACHINE_MAX) return;
    m_machines.push_back(std::make_shared<OmniMachine>(machineName,
        OmniConfig::GetInstance()->GetCommand(machineName), m_virtualTerminalRows, m_virtualTerminalCols));
}

void OmniMachineManager::AddMachinesFromGroup(const MachineGroup &machineGroup)
{
    auto iter = m_machineGroups.find(machineGroup);
    if (iter == m_machineGroups.end()) {
        LOG4CPLUS_WARN_FMT(omnitty::LOGGER_NAME, "cannot find group: %s %lu", machineGroup.c_str(), machineGroup.size());
        std::for_each(m_machineGroups.begin(), m_machineGroups.end(), [&](const std::pair<MachineGroup, std::set<MachineIp>> &groups){
            LOG4CPLUS_INFO_FMT(omnitty::LOGGER_NAME, "load [%s] machine count: %lu", groups.first.c_str(), groups.second.size());
        });
        return;
    }
    std::for_each(iter->second.begin(), iter->second.end(), [&](const std::string &ip){
        AddMachine(ip);
    });
}


void OmniMachineManager::DeleteCurrentMachine()
{
    DeleteMachineByIndex(m_selectedMachine);
}


void OmniMachineManager::DeleteTaggedMachines()
{
    auto iter = m_machines.begin();
    while (iter != m_machines.end()) {
        if ((*iter)->IsTagged()) {
            m_machines.erase(iter++);
            continue;
        }
        ++iter;
    }
}


void OmniMachineManager::DeleteAllMachines()
{
    m_machines.clear();
    m_selectedMachine = 0;
    m_scrollPos = 0;
}


void OmniMachineManager::DeleteDeadMachines()
{
    auto iter = m_machines.begin();
    while (iter != m_machines.end()) {
        if ((*iter)->IsAlive()) {
            m_machines.erase(iter++);
            continue;
        }
        ++iter;
    }
}


void OmniMachineManager::RenameMachine(const std::string &newName)
{
    m_machines[m_selectedMachine]->SetMachineName(newName);
    UpdateAllMachines();
}


void OmniMachineManager::UpdateAllMachines()
{
    for (auto &machine : m_machines) {
        rote_vt_update(machine->GetVirtualTerminal());
    }
}


void OmniMachineManager::ResetSelectedMachine(int height)
{
    /* clamp m_selectedMachine to bounds */
    if (m_selectedMachine >= static_cast<int>(m_machines.size())) {
        m_selectedMachine = m_machines.size() - 1;
    }

    /* in particular, if machcount == 0, m_selectedMachine will be 0 */
    if (m_selectedMachine < 0) {
        m_selectedMachine = 0;
    }

    /* correct scrolling if needed */
    if (m_machines.size() > 0) {
        if (m_selectedMachine < m_scrollPos) {
            m_scrollPos = m_selectedMachine;
        }
        if (m_selectedMachine >= m_scrollPos + height) {
            m_scrollPos = m_selectedMachine - height + 1;
        }
    }
}


std::string OmniMachineManager::MakeVirtualTerminalSummary(uint32_t machineIndex, int summaryWidth)
{
    if (machineIndex >= m_machines.size()) return std::string();
    MachinePtr machine = m_machines[machineIndex];
    RoteTerm *rt = machine->GetVirtualTerminal();

    std::string summary(summaryWidth, '\0');
    int r = rt->crow;
    int c = rt->ccol;
    for (int i = summaryWidth - 2; i >= 0; --i) {
        if (r > 0) {
            summary[i] = rt->cells[r][c].ch;
            if ((static_cast<int>(summary[i]) >= 0 && static_cast<int>(summary[i]) < 32) ||
                    static_cast<int>(summary[i]) == 127) {
                summary[i] = 32;
            }
        } else {
            summary[i] = 32;
        }

        if (--c < 0) {
            c = rt->cols - 1;
            if (--r > 0) {
                while (c > 0 && rt->cells[r][c-1].ch == 32) {
                    --c;
                }
            }
        }
    }
    return summary;
}


void OmniMachineManager::TagCurrent()
{
    if (m_selectedMachine >= 0 && m_selectedMachine < static_cast<int>(m_machines.size())) {
        m_machines[m_selectedMachine]->ToggleIsTagged();
    }
}


void OmniMachineManager::TagAll(bool ignoreDead)
{
    for (auto &machine : m_machines) {
        machine->SetIsTagged(!ignoreDead || machine->IsAlive());
    }
}


void OmniMachineManager::UnTagAll()
{
    for (auto &machine : m_machines) {
        machine->SetIsTagged(false);
    }
}


void OmniMachineManager::PrevMachine()
{
    --m_selectedMachine;
}


void OmniMachineManager::NextMachine()
{
    ++m_selectedMachine;
}

void OmniMachineManager::ForwardKeypress(int key)
{
    if (m_isMulticast) {
        std::for_each(m_machines.begin(), m_machines.end(), [&](std::shared_ptr<OmniMachine> &machine){
            if (machine->IsTagged()) rote_vt_keypress(machine->GetVirtualTerminal(), key);
        });
        return;
    }

    if (m_selectedMachine >= 0 && m_selectedMachine < static_cast<int>(m_machines.size()))
        rote_vt_keypress(m_machines[m_selectedMachine]->GetVirtualTerminal(), key);
}


void OmniMachineManager::HandleDeath(pid_t pid)
{
    for (auto &machine : m_machines) {
        if (machine->GetPid() == pid) {
            machine->SetIsAlive(false);
            rote_vt_forsake_child(machine->GetVirtualTerminal());
            break;
        }
    }
}


void OmniMachineManager::DeleteMachineByIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_machines.size()))
        return;
    m_machines.erase(m_machines.begin() + index);
}

