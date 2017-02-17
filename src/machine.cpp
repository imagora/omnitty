#include "machine.h"


#define CMD_FORMAT "/usr/bin/ssh %s"
#define TAGSTACK_SIZE 8


using namespace omnitty;


OmniMachine::OmniMachine(const std::string &machineName, int vtRows, int vtCols)
    : m_isTagged(false), m_isAlive(true), m_machineName(machineName)
{
    m_tagStack.reserve(TAGSTACK_SIZE);
    m_virtualTerminal = rote_vt_create(vtRows, vtCols);

    /* build the command line and fork an ssh to the given machine */
    static char cmd[128];
//    if (120 < snprintf(cmd, 120, CMD_FORMAT, m->name)) abort();
    m_pid = rote_vt_forkpty(m_virtualTerminal, cmd);
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

