#pragma once
#include <set>
#include <map>
#include <list>
#include <memory>
#include <sys/types.h>
#include <ncurses.h>
#include "machine.h"


namespace omnitty {


typedef std::string MachineGroup;
typedef std::string MachineIp;
typedef std::string MachineName;


class OmniMachineInfo
{
public:
    OmniMachineInfo() = default;

    OmniMachineInfo(const OmniMachineInfo &info);

    OmniMachineInfo(OmniMachineInfo &&info);

    ~OmniMachineInfo() = default;

    OmniMachineInfo &operator=(OmniMachineInfo &&info);

    const MachineName &GetMachineName() const { return m_machineName; }

    const MachineIp &GetMachineIp() const { return m_machineIp; }

    bool SetMachineInfo(const std::string &infoStr);

    bool IsMatchFuzzySearch(const std::string &searchInfo) const;

private:
    MachineName m_machineName;
    MachineIp   m_machineIp;
};


inline bool operator<(const OmniMachineInfo &lhv, const OmniMachineInfo &rhv)
{
    return lhv.m_machineName < rhv.m_machineName ||
        (!(lhv.m_machineName < rhv.m_machineName) && lhv.m_machineIp < rhv.m_machineIp);
}

typedef std::shared_ptr<OmniMachine>                      MachinePtr;
typedef std::vector<MachinePtr>                           MachineList;
typedef std::map<MachineGroup, std::set<OmniMachineInfo>> MachineGroups;


/**
 * @brief The MachineManager class
 */
class OmniMachineManager
{
public:
    /**
     * @brief Initialize the machine manager.
     * @details The given window will be used as the "list window", that is, the
     *          window that will display the list of the available machines. The
     *          vtrows, vtcols arguments specify the size of the virtual terminal
     *          of the machines. It needs to know this because it will handle
     *          machine creation when you call AddMachine() for example, and it
     *          needs to pass those dimensions to the machine-creating functions
     *          (e.g. machine_new)
     */
    OmniMachineManager();
    
    
    /**
     * @brief Release all members.
     */
    ~OmniMachineManager();


    int GetSelectedMachine() const { return m_selectedMachine; }


    int GetScrollPos() const { return m_scrollPos; }


    uint32_t GetMachineCount() const { return static_cast<uint32_t>(m_machines.size()); }


    MachinePtr GetMachine(uint32_t index) { return index > m_machines.size() ? nullptr : m_machines[index]; }


    void SetVirtualTerminalSize(uint32_t virtualTerminalRows, uint32_t virtualTerminalCols) {
        m_virtualTerminalRows = virtualTerminalRows;
        m_virtualTerminalCols = virtualTerminalCols;
    }


    bool LoadMachines(const std::string &fileName);
    
    
public:
    /**
     * @brief Adds a new machine to the machine manager given its name.
     * @details Takes care of creating the machine and adding it to the list.
     * @param machineName Machine's name.
     * @param machineIp Machine's ip.
     */
    int AddMachine(const std::string &machineName, const std::string &machineIp);


    void AddMachinesFromGroup(const MachineGroup &machineGroup);


    /**
     * @brief Deletes currently selected machine.
     */
    void DeleteCurrentMachine();


    /**
     * @brief Deletes all tagged machines.
     */
    void DeleteTaggedMachines();


    /**
     * @brief Deletes all the machines!
     */
    void DeleteAllMachines();


    /**
     * @brief Deletes all dead machines, that is, all machines whose 'alive'.
     *        flag is false.
     * @details A machine's 'alive' flag is dropped when you call HandleDeath(pid)
     *          with a pid matching that machine's child ssh process.
     *          Ordinarily the main program should monitor SIGCHLD signals and call
     *          machmgr_handle_death appropriately when it detects the death of a
     *          child process.
     */
    void DeleteDeadMachines();


    /**
     * @brief Rename the currently selected machine.
     * @param newName new name for machine
     */
    void RenameMachine(const std::string &newName);
    
    
    /**
     * @brief Updates the virtual terminals of all machines.
     * @details This function should be called regularly.
     */
    void UpdateAllMachines();


    void ResetSelectedMachine(int height);


    /**
     * @brief MakeVirtualTerminalSummary)
     * @param machineIndex machine's index
     * @param summaryWidth the summary window width
     * @return summary
     */
    std::string MakeVirtualTerminalSummary(uint32_t machineIndex, int summaryWidth);
    

public:
    /**
     * @brief Toggles multicast mode.
     * @details When multicast mode is on, machmgr_forward_keypress will send
     *          the keypress to all tagged machines; when multicast mode is off,
     *          machmgr_forward_keypress will send keypresses only to the
     *          currently selected machine.
     *
     *          Multicast mode is initially off.
     */
    void ToggleMulticast() { m_isMulticast = !m_isMulticast; }
    

    /**
     * @brief Get multicast mode.
     * @return Whether multicast mode is on.
     */
    bool IsMulticast() { return m_isMulticast; }


    /**
     * @brief Toggles the 'tagged' state of the currently selected machine.
     */
    void TagCurrent();


    /**
     * @brief Tags all machines.
     * @details If ignore_dead, does not tag dead machines (i.e. machines whose
     *          alive flag is down).
     * @param ignoreDead whether to ignore dead machines
     */
    void TagAll(bool ignoreDead);


    /**
     * @brief Resets the 'tagged' attribute of all machines.
     */
    void UnTagAll();


    /**
     * @brief Moves the selection to the previous machine.
     * @details Makes the previous machine in the list the selected machine.
     */
    void PrevMachine();


    /**
     * @brief Moves to the next machine.
     * @details Makes the next machine in the list the selected machine.
     */
    void NextMachine();


    /**
     * @brief Forwards the given keypress to the appropriate machines.
     * @details If multicast mode is on, the keypress will be forwarded to all
     *          tagged machines; otherwise, it will be directed only to the
     *          currently selected machine.
     * @param key the pressed key
     */
    void ForwardKeypress(int key);


    /**
     * @brief Handles the death of PID p.
     * @details This will check if that PID matches the PID of the child ssh
     *          process of any of the machines registered in the manager. If so,
     *          it will mark that machine as dead.
     * @param pid the machine's pid
     */
    void HandleDeath(pid_t pid);


    void SendCommand(int machineId, const std::string &cmd);
    

protected:
    /**
     * @brief DeleteMachineByIndex
     * @param index the machine's index
     */
    void DeleteMachineByIndex(int index);


protected:
    OmniMachineManager(const OmniMachineManager &) = delete;
    OmniMachineManager &operator=(const OmniMachineManager &) = delete;
    

private:
    /* whether keystrokes are sent to all tagged machines or not */
    bool                m_isMulticast;
    /* currently selected machine */
    int                 m_selectedMachine;
    /* machine being shown at the top of the list */
    int                 m_scrollPos;
    uint32_t            m_virtualTerminalRows;
    uint32_t            m_virtualTerminalCols;
    MachineList         m_machines;
    MachineGroups       m_machineGroups;
};


}

