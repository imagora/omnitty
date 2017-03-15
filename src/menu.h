#pragma once
#include <memory>
#include <ncurses.h>


namespace omnitty {


class OmniMachineManager;
typedef std::shared_ptr<OmniMachineManager> MachineManagerPtr;


class OmniMenu
{
public:
    /**
     * @brief Initializes the menu system.
     * @param machineMgrPtr The machine manager ptr.
     */
    OmniMenu(MachineManagerPtr machineMgrPtr);


    ~OmniMenu();


    void InitMenu(int rows, int cols);


    void DrawMenu();


    /**
     * @brief Shows the Omnitty extended menu on screen.
     * @details After calling this function, the screen will be dirty so you
     *          must touchwin() all your windows to get them to redraw fully.
     */
    void ShowMenu();


    void UpdateCastLabel();


    /* caution: the following function uses what is already in the buffer
     * as the default content of the field, so it absolutely must not
     * contain trash (and must be 0-terminated).
     *
     * If you want to start with an empty buffer, do something like *buf = 0
     * before calling.
     *
     * Returns whether used confirmed the input, that is whether the
     * user ended the input with ENTER. If the user exitted with ESC
     * any other 'cancel-key', the return value will be false.
     */
    bool Prompt(const char *prompt, unsigned char attr, char *buf, int len);
    bool Prompt(const char *prompt, unsigned char attr, char *buf[], int len, int size);


    /**
     * @brief Displays a message in the window.
     * @details Waits for the user to acknowledge it and returns.
     * @param msg
     * @param attr
     */
    void ShowMessageAndWait(const char *msg, unsigned char attr);


    /**
     * @brief Displays a message in the minibuffer and returns immediately.
     * @details Remember to call this function with an empty (or NULL) message
     *          to erase it.
     * @param msg
     * @param attr
     */
    void ShowMessageNotWait(const char *msg, unsigned char attr);


protected:
    /**
     * @brief Shows a help window on screen.
     * @details After calling this function, you should touchwin() your windows
     *          to make them redraw fully the next time, since your screen will
     *          be trashed.
     */
    void ShowHelp();


private:
    WINDOW              *m_menuWnd;
    MachineManagerPtr   m_machineMgr;
};


}

