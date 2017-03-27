#pragma once
#include <map>
#include <string>
#include "menu.h"


namespace omnitty {


class OmniWindowManager
{
    typedef void (OmniWindowManager::*KeypressFuncPtr)();

public:
    OmniWindowManager();


    ~OmniWindowManager();


    WINDOW *GetListWnd() const { return m_listWnd; }


    WINDOW *GetVirtualTerminalWnd() const { return m_virtualTerminalWnd; }


    WINDOW *GetSummaryWnd() const { return m_summaryWnd; }


public:
    /**
     * @brief Init the window manager.
     */
    void Init();


    void LoadMachines();


    /**
     * @brief Updates the virtual terminals of all machines.
     * @details This function should be called regularly.
     */
    void UpdateAllMachines();

    /**
     * @brief Handles the death of PID p.
     * @details This will check if that PID matches the PID of the child ssh
     *          process of any of the machines registered in the manager. If so,
     *          it will mark that machine as dead.
     * @param pid the pid of the machine
     */
    void HandleDeath(pid_t pid);

    /**
     * @brief Keypress, handle F1 - F7 and send others to terminal.
     * @param key the pressed key.
     */
    void Keypress(int key);

private:
    /**
     * @brief Init Windows
     * @details Window layout:
     *
     *             list    summary     terminal window
     *            window   window
     *          |-------|--------X|--------------------------------|
     *          0       A        BC                             termcols-1
     *
     *          A = list_win_chars + 2
     */
    void DrawWindows();

    /**
     * @brief Draws the machine list onto the list window.
     */
    void DrawMachineList();

    /**
     * @brief Draws the summary area in the passed window.
     * @details The "summary" consists of a few characters for each machine, and
     *          those characters give the user an idea of what is going on in
     *          that machine's terminal. Those characters will be the characters
     *          that are "near" the cursor. This is difficult to define, so see
     *          the implementation :-)
     *
     *          The supplied window should be on either side of the list window,
     *          and vertically aligned with it (i.e. its top should match the list
     *          window's top, same for bottom), because this function will draw
     *          the summaries that correspond to each machine in the list window
     *          in the corresponding line of the supplied window.
     *
     *           list win     summary window (the one you supply to this function)
     *           +----------+--------------------------+
     *           | mach 1   | summary for machine 1    |
     *           | mach 2   | summary for machine 2    |
     *           | mach 3   | summary for machine 3    |
     *           | ...      | ...                      |
     *           +----------+--------------------------+
     */
    void DrawSummary();

    /**
     * @brief Draws the virtual terminal for the currently selected machine in
     *        the given window.
     * @details Assumes the dimensions of the given window match the vtrows,
     *          vtcols arguments passed to constructor.
     */
    void DrawVirtualTerminal();

    /**
     * @brief Redraw
     * @param forceFullRedraw whether to force full redraw
     */
    void Redraw(bool forceFullRedraw);

private:
    /**
     * @brief ShowMenu, for F1 keypress.
     */
    void ShowMenu();

    /**
     * @brief Moves the selection to the previous machine, for F2 keypress.
     * @details Makes the previous machine in the list the selected machine.
     */
    void PrevMachine();

    /**
     * @brief Moves to the next machine, for F3 keypress.
     * @details Makes the next machine in the list the selected machine.
     */
    void NextMachine();

    /**
     * @brief Toggles the 'tagged' state of the currently selected machine,
     *        for F4 key press.
     */
    void TagCurrent();

    /**
     * @brief Add machine, for F5 keypress.
     */
    void AddMachine();

    void AddMachinesFromFile(const std::string &file);

    void AddMachinesFromGroup(const std::string &group);

    /**
     * @brief Delete machine, for F6 keypress.
     */
    void DeleteMachine();

    /**
     * @brief Toggle multicast, for F7 keypress.
     */
    void ToggleMulticast();

    /**
     * @brief Forwards the given keypress to the appropriate machines.
     * @details If multicast mode is on, the keypress will be forwarded to all
     *          tagged machines; otherwise, it will be directed only to the
     *          currently selected machine.
     * @param key the pressed key
     */
    void ForwardKeypress(int key);

    void SelectMachine();

private:
    int                             m_listWndWidth;
    int                             m_summaryWndWidth;
    int                             m_terminalWndWidth;
    WINDOW                          *m_listWnd;
    WINDOW                          *m_virtualTerminalWnd;
    WINDOW                          *m_summaryWnd;
    MachineManagerPtr               m_machineMgr;
    OmniMenu                        m_menu;
    std::map<int, KeypressFuncPtr>  m_keypressFuncPtrs;
};


}

