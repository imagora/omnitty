#pragma once
#include <list>
#include <memory>
#include <sys/types.h>
#include <ncurses.h>
#include "machine.h"


typedef std::shared_ptr<Machine>    MachinePtr;
typedef std::vector<MachinePtr>     MachineList;


/**
 * @brief The MachineManager class
 */
class MachineManager
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
     * @param listWnd The list window handler.
     * @param vtrows The rows of the virtual terminal.
     * @param vtcols the cols of the virtual terminal.
     */
    MachineManager();
    
    
    /**
     * @brief Release all members.
     */
    ~MachineManager();


    int GetSelectedMachine() const { return m_selectedMachine; }


    int GetScrollPos() const { return m_scrollPos; }


    uint32_t GetMachineCount() const { return static_cast<uint32_t>(m_machines.size()); }


    MachinePtr GetMachine(uint32_t index) { return index > m_machines.size() ? nullptr : m_machines[index]; }


    void SetVirtualTerminalSize(uint32_t virtualTerminalRows, uint32_t virtualTerminalCols) {
        m_virtualTerminalRows = virtualTerminalRows;
        m_virtualTerminalCols = virtualTerminalCols;
    }
    
    
public:
    /**
     * @brief Adds a new machine to the machine manager given its name.
     * @details Takes care of creating the machine and adding it to the list.
     * @param machineName Machine's name.
     */
    void AddMachine(const std::string &machineName);


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
     * @param newname
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
     * @param rt
     * @param summaryWidth
     * @return
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
     * @param ignoreDead
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
     * @param k
     */
    void ForwardKeypress(int key);


    /**
     * @brief Handles the death of PID p.
     * @details This will check if that PID matches the PID of the child ssh
     *          process of any of the machines registered in the manager. If so,
     *          it will mark that machine as dead.
     * @param p
     */
    void HandleDeath(pid_t pid);
    

protected:
    /**
     * @brief DeleteMachineByIndex
     * @param index
     */
    void DeleteMachineByIndex(int index);


protected:
    MachineManager(const MachineManager &) = delete;
    MachineManager &operator=(const MachineManager &) = delete;
    

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
};

