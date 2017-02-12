#include <ncurses.h>
#include <stdlib.h>
#include "curutil.h"
#include "help.h"
#include "machine_manager.h"
#include "menu.h"


#define MENU_LINES 12
#define MENU_COLS  38

#define MENU_CONTENTS \
"{[h]} online help\n" \
"{[r]} rename machine\n" \
"{[t]} tag all machines (live only)\n" \
"{[T]} tag all machines (live & dead)\n" \
"{[u]} untag all machines\n" \
"{[z]} delete dead machines\n" \
"{[d]} delete all TAGGED machines\n" \
"{[X]} delete all machines\n" \
"{[q]} quit application\n"


OmniMenu::OmniMenu(MachineManagerPtr machineMgrPtr)
    : m_machineMgr(machineMgrPtr)
{

}


OmniMenu::~OmniMenu()
{

}


void OmniMenu::InitMenu(int rows, int cols)
{
    m_menuWnd = newwin(1, rows, cols, 0);
}


void OmniMenu::DrawMenu()
{
    touchwin(m_menuWnd);
    wclear(m_menuWnd);
}


void OmniMenu::ShowMenu()
{
    int termwidth, termheight;
    char buf[16];
    const char *p; int i; int ch;
    WINDOW *w;

    getmaxyx(stdscr, termheight, termwidth);
    w = newwin(MENU_LINES, MENU_COLS, (termheight - MENU_LINES)/2,
               (termwidth  - MENU_COLS )/2);
    werase(w);
    curutil_attrset(w, 0xF4);
    curutil_window_fill(w, ' ');

    wborder(w, 0, 0, 0, 0, 0, 0, 0, 0);
    wmove(w, 0, 3); waddstr(w, "[ Menu ]");

    wmove(w, i = 1, 1);
    p = MENU_CONTENTS;
    while (*p) {
        if      (*p == '{')  curutil_attrset(w, 0xE4);
        else if (*p == '}')  curutil_attrset(w, 0xF4);
        else if (*p == '\n') wmove(w, ++i, 1);
        else                 waddch(w, *p);
        p++;
    }

    wmove(w, ++i, 1);
    curutil_attrset(w, 0xE4);
    waddstr(w, "Select an option: ");
    wrefresh(w);

    while (0 > (ch = getch())) ;

    switch (ch) {
    case 'h': help_show(); break;
    case 'r':
        *buf = 0;
        Prompt("Enter new machine name: ", 0x90, buf, 15);
        m_machineMgr->RenameMachine(buf);
        break;
    case 'q': *buf = 0;
        if (Prompt("Really quit application [y/n]?", 0x90, buf, 2) && (*buf == 'y' || *buf == 'Y')) {
            endwin();
            exit(0);
        }
        break;
    case 't':
        m_machineMgr->TagAll(true);
        break;
    case 'T':
        m_machineMgr->TagAll(false);
        break;
    case 'u':
        m_machineMgr->UnTagAll();
        break;
    case 'z':
        m_machineMgr->DeleteDeadMachines();
        break;
    case 'd':
        *buf = 0;
        if (Prompt("Really delete all TAGGED machines [y/n]?", 0x90, buf, 2) && (*buf == 'y' || *buf == 'Y')) {
            m_machineMgr->DeleteTaggedMachines();
        }
        break;
    case 'X': *buf = 0;
        if (Prompt("Really delete ALL machines [y/n]?", 0x90, buf, 2) && (*buf == 'y' || *buf == 'Y')) {
            m_machineMgr->DeleteAllMachines();
        }
        break;
    }

    delwin(w);
}


void OmniMenu::UpdateCastLabel()
{
    /* draws the label that says 'single cast' or 'multicast' on minibuffer */
    int termwidth, termheight;
    const char *msg;
    getmaxyx(m_menuWnd, termheight, termwidth);
    if (m_machineMgr->IsMulticast()) {
        curutil_attrset(m_menuWnd, 0xF9); /* bright blinking white over red */
        msg = "!!! MULTICAST MODE !!!";
    }
    else {
        curutil_attrset(m_menuWnd, 0x40);
        msg = "singlecast mode";
    }

    werase(m_menuWnd);
    wmove(m_menuWnd, 0, termwidth - strlen(msg));
    waddstr(m_menuWnd, msg);

    leaveok(m_menuWnd, TRUE);  /* prevent cursor movement */
    wrefresh(m_menuWnd);
    leaveok(m_menuWnd, FALSE);
}


bool OmniMenu::Prompt(const char *prompt, unsigned char attr, char *buf, int len)
{
    int pos = strlen(buf);
    int decision = 0;
    while (!decision) {
        werase(m_menuWnd);
        wmove(m_menuWnd, 0, 0);
        curutil_attrset(m_menuWnd, attr);
        waddstr(m_menuWnd, prompt);
        curutil_attrset(m_menuWnd, 0x70);
        waddstr(m_menuWnd, buf);
        wrefresh(m_menuWnd);

        int ch = getch();
        if (ch < 0) continue;

        switch (ch) {
        case 127:
        case KEY_BACKSPACE:
        case '\b':
            /* bs */
            if (pos) pos--;
            break;
        case ('U'-'A'+1):
            /* ^U */
            pos = 0;
            break;
        case '\r':
        case '\n':
            /* Enter */
            decision = 1;
            break;
        case ('C'-'A'+1):
        case ('G'-'A'+1):
        case 0x1B:
            /* cancel */
            decision = -1;
            break;
        }

        if (ch >= 32 && ch != 127 && ch < 256 && pos + 1 < len) {
            /* regular char, and we have room: put it in the buffer */
            buf[pos++] = ch;
        }

        buf[pos] = 0;
    }

    werase(m_menuWnd);
    wrefresh(m_menuWnd);
    return decision == 1;
}


void OmniMenu::ShowMessageAndWait(const char *msg, unsigned char attr)
{
    ShowMessageNotWait(msg, attr);
    wrefresh(m_menuWnd);
    while (ERR == getch()) ;
    wclear(m_menuWnd);
}


void OmniMenu::ShowMessageNotWait(const char *msg, unsigned char attr)
{
    werase(m_menuWnd);
    wmove(m_menuWnd, 0, 0);
    if (msg) {
        curutil_attrset(m_menuWnd, attr);
        waddstr(m_menuWnd, msg);
    }
    wrefresh(m_menuWnd);
}

