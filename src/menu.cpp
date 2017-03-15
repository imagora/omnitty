#include <ncurses.h>
#include <stdlib.h>
#include "config.h"
#include "curutil.h"
#include "machine_manager.h"
#include "menu.h"


using namespace omnitty;


#define MENU_LINES 12
#define MENU_COLS  38


static const std::string MENU_CONTENTS(
        "{[h]} online help\n"
        "{[r]} rename machine\n"
        "{[t]} tag all machines (live only)\n"
        "{[T]} tag all machines (live & dead)\n"
        "{[u]} untag all machines\n"
        "{[z]} delete dead machines\n"
        "{[d]} delete all TAGGED machines\n"
        "{[X]} delete all machines\n"
        "{[q]} quit application\n");


#define HELP_LINES 25
#define HELP_COLS  80


static const std::string HELP_CONTENTS(
        "Omnitty allows you to ssh into several machines at once. The list on the\n"
        "shows the machines you are logged into. Every key you type will be sent\n"
        "to the currently selected machine, except the following {control keys}:\n"
        "\n"
        "{F1}: brings up a menu of extended options (including {quit})\n"
        "{F2} and {F3}: move between machines on the list. The terminal window\n"
        "      on the right shows the display of the currently selected machine.\n"
        "{F4}: tag current machine. This will put a mark on the machine's name.\n"
        "      The mark by itself does nothing, but some features operate on\n"
        "      'all tagged machines', so this is a way to indicate which machines\n"
        "      they are to operate on. The {F1} menu has additional tag commands.\n"
        "{F5}: allows you to add a machine to the list. You can either supply the\n"
        "      name of the machine you want to add, or type {@file} where {file}\n"
        "      is the name of a file that contains machine names one per line,\n"
        "      in which case all machines in that file will be added.\n"
        "{F6}: delete current machine. If you want to delete multiple machines,\n"
        "      you can also tag them and choose {delete all tagged machines} from\n"
        "      the {F1} menu.\n"
        "{F7}: toggles between {singlecast} and {multicast} mode. In singlecast\n"
        "      mode, the keys you type will be directed to the currently selected\n"
        "      machine, regardless of tags. When {multicast} is selected, the\n"
        "      keys will be sent to {all tagged machines}, allowing you to operate\n"
        "      on several machines at once.\n");


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
    getmaxyx(stdscr, termheight, termwidth);
    WINDOW *w = newwin(MENU_LINES, MENU_COLS, (termheight - MENU_LINES)/2,
               (termwidth  - MENU_COLS )/2);
    werase(w);
    CurutilAttrset(w, 0xF4);
    omnitty::CurutilWindowFill(w, ' ');

    wborder(w, 0, 0, 0, 0, 0, 0, 0, 0);
    wmove(w, 0, 3); waddstr(w, "[ Menu ]");

    int i;
    wmove(w, i = 1, 1);
    const char *p = MENU_CONTENTS.c_str();
    while (*p) {
        if      (*p == '{')  CurutilAttrset(w, 0xE4);
        else if (*p == '}')  CurutilAttrset(w, 0xF4);
        else if (*p == '\n') wmove(w, ++i, 1);
        else                 waddch(w, *p);
        p++;
    }

    wmove(w, ++i, 1);
    CurutilAttrset(w, 0xE4);
    waddstr(w, "Select an option: ");
    wrefresh(w);

    int ch;
    while (0 > (ch = getch())) ;

    char buf[16];
    switch (ch) {
    case 'h': ShowHelp(); break;
    case 'r':
        *buf = 0;
        Prompt("Enter new machine name: ", 0x90, buf, 15);
        m_machineMgr->RenameMachine(buf);
        break;
    case 'q': *buf = 0;
        if (Prompt("Really quit application [y/n]?", 0x90, buf, 2) && (*buf == 'y' || *buf == 'Y')) {
            OmniConfig::GetInstance()->SaveConfig();
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
        CurutilAttrset(m_menuWnd, 0xF9); /* bright blinking white over red */
        msg = "!!! MULTICAST MODE !!!";
    }
    else {
        CurutilAttrset(m_menuWnd, 0x40);
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
        CurutilAttrset(m_menuWnd, attr);
        waddstr(m_menuWnd, prompt);
        CurutilAttrset(m_menuWnd, 0x70);
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

bool OmniMenu::Prompt(const char *prompt, unsigned char attr, char *buf[], int len, int size)
{
    int sPos = 0;
    int pos = strlen(buf[sPos]);
    int decision = 0;
    while (!decision) {
        werase(m_menuWnd);
        wmove(m_menuWnd, 0, 0);
        CurutilAttrset(m_menuWnd, attr);
        waddstr(m_menuWnd, prompt);
        CurutilAttrset(m_menuWnd, 0x70);
        waddstr(m_menuWnd, buf[sPos]);
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

        case 32:
            ++sPos;
            pos = strlen(buf[sPos]);
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
            buf[sPos][pos++] = ch;
        }

        buf[sPos][pos] = 0;
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
        CurutilAttrset(m_menuWnd, attr);
        waddstr(m_menuWnd, msg);
    }
    wrefresh(m_menuWnd);
}

void OmniMenu::ShowHelp()
{
    int termwidth, termheight;
    getmaxyx(stdscr, termheight, termwidth);
    WINDOW *w = newwin(HELP_LINES, HELP_COLS, (termheight - HELP_LINES)/2,
               (termwidth  - HELP_COLS )/2);
    werase(w);
    CurutilAttrset(w, 0xF4);
    CurutilWindowFill(w, ' ');

    wborder(w, 0, 0, 0, 0, 0, 0, 0, 0);
    wmove(w, 0, 3); waddstr(w, "[ Help ]");

    int i;
    wmove(w, i = 1, 1);
    const char *p = HELP_CONTENTS.c_str();
    while (*p) {
        if      (*p == '{')  CurutilAttrset(w, 0xE4);
        else if (*p == '}')  CurutilAttrset(w, 0xF4);
        else if (*p == '\n') wmove(w, ++i, 1);
        else                 waddch(w, *p);
        p++;
    }

    wrefresh(w);
    int ch;
    while (0 > (ch = getch())) ;
    delwin(w);
}

