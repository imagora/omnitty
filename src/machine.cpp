#include "machine.h"


#define TAGSTACK_SIZE 8


using namespace omnitty;


OmniMachine::OmniMachine(const std::string &machineName, const std::string &machineIp, const std::string &command,
                         int vtRows, int vtCols)
    : m_isTagged(false), m_isAlive(true), m_machineName(machineName), m_machineIp(machineIp)
{
    m_tagStack.reserve(TAGSTACK_SIZE);
    m_virtualTerminal = rote_vt_create(vtRows, vtCols);
    m_pid = rote_vt_forkpty(m_virtualTerminal, command.c_str());
}


OmniMachine::~OmniMachine()
{
    rote_vt_destroy(m_virtualTerminal);
}


void OmniMachine::PushMachineTag()
{
    if (m_tagStack.size() >= TAGSTACK_SIZE) return;
    m_tagStack.push_back(static_cast<uint8_t>(m_isTagged));
}


void OmniMachine::PopMachineTag()
{
    if (m_tagStack.empty()) return;
    m_isTagged = static_cast<bool>(m_tagStack.back());
    m_tagStack.pop_back();
}

