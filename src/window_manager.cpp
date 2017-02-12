#include "curutil.h"
#include "machine_manager.h"
#include "window_manager.h"


/* minimum terminal dimensions to run program */
#define MIN_REQUIRED_WIDTH 80
#define MIN_REQUIRED_HEIGHT 25


#define OMNITTY_VERSION "0.4.0"
#define REMINDER_LINE "OmNiTTY-R v" OMNITTY_VERSION \
"  \007F1\007:menu  \006F2/3\007:sel  \003F4\007:tag" \
"  \002F5\007:add  \001F6\007:del" \
"  \005F7\007:mcast"


#define SPLASH_LINE_1 "OmNiTTY Agora v" OMNITTY_VERSION
#define SPLASH_LINE_2 "Copyright (c) 2017 AgoraLab"




WindowManager::WindowManager(int listWndWidth, int terminalWndWidth)
    : m_listWndWidth(listWndWidth), m_terminalWndWidth(terminalWndWidth),
      m_machineMgr(std::make_shared<MachineManager>()), m_menu(m_machineMgr)
{
}


WindowManager::~WindowManager()
{

}


void WindowManager::InitCurses()
{
    initscr();
    start_color();
    noecho();
    keypad(stdscr, TRUE);
    timeout(200);
    raw();
    curutil_colorpair_init();
    clear();

    /* register some alternate escape sequences for the function keys,
     * to improve compatibility with several types of terminal emulators */
    define_key("\eOP",   KEY_F(1));
    define_key("\eOQ",   KEY_F(2));
    define_key("\eOR",   KEY_F(3));
    define_key("\eOS",   KEY_F(4));
    define_key("\e[11~", KEY_F(1));
    define_key("\e[12~", KEY_F(2));
    define_key("\e[13~", KEY_F(3));
    define_key("\e[14~", KEY_F(4));
    define_key("\e[15~", KEY_F(5));
    define_key("\e[17~", KEY_F(6));
    define_key("\e[18~", KEY_F(7));
    define_key("\e[19~", KEY_F(8));
    define_key("\e[20~", KEY_F(9));
    define_key("\e[21~", KEY_F(10));

    int w, h, i = 0;
    getmaxyx(stdscr, h, w);
    if (h < MIN_REQUIRED_HEIGHT || w < MIN_REQUIRED_WIDTH) {
        endwin();
        fprintf(stderr, "ERROR: omnitty requires a %d x %d terminal to run.\n",
                MIN_REQUIRED_WIDTH, MIN_REQUIRED_HEIGHT);
        exit(1);
    }

    wmove(stdscr, h / 2, (w - strlen(SPLASH_LINE_1))/2);
    curutil_attrset(stdscr, 0x40);
    waddstr(stdscr, SPLASH_LINE_1);

    curutil_attrset(stdscr, 0x70);
    wmove(stdscr, h/2 + 1, (w - strlen(SPLASH_LINE_2))/2);
    waddstr(stdscr, SPLASH_LINE_2);

    wrefresh(stdscr);
    while (getch() < 0 && i < 10) i++;

    wclear(stdscr);
    wrefresh(stdscr);
}


/* Window layout:
 *
 *      list    summary     terminal window
 *     window   window
 *    |-------|--------X|--------------------------------|
 *    0       A        BC                             termcols-1
 *
 * A = list_win_chars + 2
 */
void WindowManager::InitWindows()
{
    /* obtain terminal dimensions */
    int termcols, termrows;
    getmaxyx(stdscr, termrows, termcols);

    /* the geometry is hard-coded here, but nowhere else... so I don't
     * see a lot of point using #defines or anything any more sophisticated */
    int A = m_listWndWidth + 2;
    int C = termcols - m_terminalWndWidth;
    int B = C - 1;
    if (B < A) {
        B = A;
        C = B + 1;
    }

    int vtrows = termrows - 3;
    int vtcols = termcols - C;

    /* actually create the windows */
    m_listWnd = newwin(termrows - 3, A - 0, 1, 0);
    m_summaryWnd = (B - A >= 3) ? newwin(termrows-3, B - A, 1, A) : nullptr;
    m_virtualTerminalWnd = newwin(termrows-3, vtcols, 1, C);
    m_menu.InitMenu(termcols, termrows-1);

    /* draw the top decoration line */
    wattrset(stdscr, COLOR_PAIR(3) | A_BOLD);
    wmove(stdscr, 0, 0);
    whline(stdscr, ACS_HLINE | A_NORMAL, termcols);

    /* draw instruction line */
    wattrset(stdscr, COLOR_PAIR(4 * 8) | A_BOLD);
    wmove(stdscr, termrows-2, 0);
    whline(stdscr, ' ', termcols);
    wmove(stdscr, termrows-2, 0);
    const char *p = REMINDER_LINE;
    while (*p) {
        if (*p >= 0 && *p <= 7)
            wattrset(stdscr, COLOR_PAIR(4 * 8 + 7 - *p) | A_BOLD);
        else
            waddch(stdscr, *p);
        p++;
    }

    /* draw the separator at column B */
    wattrset(stdscr, COLOR_PAIR(3) | A_BOLD);
    wmove(stdscr, 0, B);
    wvline(stdscr, ACS_VLINE | A_NORMAL, termrows - 2);
    wmove(stdscr, 0, B);
    waddch(stdscr, ACS_TTEE);
    wrefresh(stdscr);

    /* draw window titles */
    if (termcols > 90) {
        wmove(stdscr, 0, 2);
        waddstr(stdscr, "[Machines]");
        wmove(stdscr, 0, B+2);
        waddstr(stdscr, "[Terminal]");
    }

    /* make the cursor position be irrelevant for all windows except
     * the terminal window */
    leaveok(m_listWnd, TRUE);
    if (m_summaryWnd) {
        leaveok(m_summaryWnd, TRUE);
    }

    /* draw all windows */
    touchwin(m_listWnd);
    wclear(m_listWnd);

    touchwin(m_virtualTerminalWnd);
    wclear(m_virtualTerminalWnd);

    if (m_summaryWnd) {
        touchwin(m_summaryWnd);
        wclear(m_summaryWnd);
    }

    m_machineMgr->SetVirtualTerminalSize(vtrows, vtcols);
    m_menu.DrawMenu();
}


void WindowManager::ShowMenu()
{
    m_menu.ShowMenu();
    SelectMachine();
}


void WindowManager::DrawMachineList()
{
    int w, h;
    werase(m_listWnd);
    getmaxyx(m_listWnd, h, w);
    int scrollPos = m_machineMgr->GetScrollPos();
    uint32_t machineCount = m_machineMgr->GetMachineCount();
    for (uint32_t i = scrollPos; i < static_cast<uint32_t>(scrollPos + h) && i < machineCount; ++i) {
        MachinePtr machine = m_machineMgr->GetMachine(i);
        /* decide color */
        unsigned char attr = machine->IsAlive() ? 0x70 : 0x80;
        if (i == static_cast<uint32_t>(m_machineMgr->GetSelectedMachine())) {
            /* red background */
            attr &= 0xF0;
            attr |= 0x01;
        }
        if (machine->IsTagged()) {
            /* green foreground */
            attr &= 0x0F;
            attr |= machine->IsAlive() ? 0xA0 : 0x20;
        }

        curutil_attrset(m_listWnd, attr);
        wmove(m_listWnd, i - scrollPos, 0);
        waddch(m_listWnd, machine->IsTagged() ? '*' : ' ');

        /* now we have to print the first w-2 characters of machine[i].name,
         * padding with spaces at the end if necessary to complete w-2
         * characters. We say w-2 because one character of the width was
         * used up when printing '*' and another one must be left blank
         * at the end */
        const char *p = machine->GetMachineName().c_str();
        int j = w - 2;
        while (j--) {
            waddch(m_listWnd, *p ? *p : ' ');
            if (*p) p++;
        }
    }
}


void WindowManager::DrawSummary()
{
    int sumheight, sumwidth;
    werase(m_summaryWnd);
    wmove(m_summaryWnd, 0, 0);
    getmaxyx(m_summaryWnd, sumheight, sumwidth);

    uint32_t scrollPos = static_cast<uint32_t>(m_machineMgr->GetScrollPos());
    uint32_t machineCount = m_machineMgr->GetMachineCount();
    for (uint32_t i = scrollPos; i < scrollPos + sumheight && i < machineCount; ++i) {
        curutil_attrset(m_summaryWnd, 0x80);
        wmove(m_summaryWnd, i - scrollPos, 0);
        std::string summary(m_machineMgr->MakeVirtualTerminalSummary(i, sumwidth));
        waddstr(m_summaryWnd, summary.c_str());
    }
}


void WindowManager::DrawVirtualTerminal()
{
    werase(m_virtualTerminalWnd);
    int selectedMachine = m_machineMgr->GetSelectedMachine();
    if (selectedMachine >= 0 && selectedMachine < static_cast<int>(m_machineMgr->GetMachineCount())) {
        MachinePtr machine = m_machineMgr->GetMachine(selectedMachine);
        rote_vt_draw(machine->GetVirtualTerminal(), m_virtualTerminalWnd, 0, 0, NULL);
    }
}


void WindowManager::Redraw(bool forceFullRedraw)
{
    if (forceFullRedraw) {
        touchwin(stdscr);
        wrefresh(stdscr);
    }

    /* draw machine list */
    DrawMachineList();
    if (forceFullRedraw) touchwin(m_listWnd);
    wrefresh(m_listWnd);

    /* draw summary window, if there is one */
    if (m_summaryWnd) {
        DrawSummary();
        if (forceFullRedraw) touchwin(m_summaryWnd);
        wrefresh(m_summaryWnd);
    }

    /* draw vt window */
    DrawVirtualTerminal();
    if (forceFullRedraw) touchwin(m_virtualTerminalWnd);
    wrefresh(m_virtualTerminalWnd);

    /* draw the 'multicast/singlecast' label */
    m_menu.UpdateCastLabel();
}


void WindowManager::AddMachine(volatile int &zombieCount)
{
    static char buf[32] = {0};
    if (m_menu.Prompt("Add: ", 0xE0, buf, 32)) {
        if (*buf == '@') {
            AddMachinesFromFile(buf+1, zombieCount);
        } else {
            m_machineMgr->AddMachine(buf);
        }
    }
    SelectMachine();
}


void WindowManager::AddMachinesFromFile(const std::string &file, volatile int &zombieCount)
{
    static char buf[128];
    bool pipe = false;
    FILE *f = nullptr;
    if (getenv("OMNITTY_AT_COMMAND")) {
        /* popen() a command */
        pipe = true;
        strcpy(buf, getenv("OMNITTY_AT_COMMAND"));
        strcat(buf, " ");
        strcat(buf, file.c_str());
        strcat(buf, " 2>/dev/null");
        f = popen(buf, "r");
    } else {
        f = fopen(file.c_str(), "r");
    }

    if (!f) {
        m_menu.ShowMessageAndWait(
                    pipe ? "Can't execute command specified by OMNITTY_AT_COMMAND" : "Can't read that file.",
                    0xF1);
        return;
    }

    m_menu.ShowMessageNotWait(
                pipe ? "Adding machines supplied by command..." : "Adding machines from file...",
                0x70);

    while (1 == fscanf(f, "%s", buf)) {
        m_machineMgr->AddMachine(buf);
    }

    if (pipe) {
        if (0 != pclose(f)) {
            m_menu.ShowMessageAndWait("Command given by OMNITTY_AT_COMMAND exited with error.", 0xF1);
        }
        /* at this point SIGCHLD will have caused zombie_count to be one more
         * than it should, since the child command has already been reaped
         * by pclose(). If we don't correct zombie_count, wait() will block
         * in the main loop, since it will try to reap a zombie that does not yet
         * exist. */
        --zombieCount;
    } else {
        fclose(f);
    }

    m_menu.ShowMessageNotWait(NULL, 0x70);
}


void WindowManager::DeleteMachine()
{
    static char buf[2] = {0};

    if (m_menu.Prompt("Really delete it [y/n]?", 0x90, buf, 2) && (*buf == 'y' || *buf == 'Y')) {
        m_machineMgr->DeleteCurrentMachine();
    }
}


void WindowManager::UpdateAllMachines()
{
    m_machineMgr->UpdateAllMachines();
}


void WindowManager::SelectMachine()
{
    int screenwidth, screenheight;
    getmaxyx(m_listWnd, screenheight, screenwidth);
    m_machineMgr->ResetSelectedMachine(screenheight);
}


void WindowManager::PrevMachine()
{
    m_machineMgr->PrevMachine();
    SelectMachine();
}


void WindowManager::NextMachine()
{
    m_machineMgr->NextMachine();
    SelectMachine();
}


void WindowManager::HandleDeath(pid_t pid)
{
    m_machineMgr->HandleDeath(pid);
}


void WindowManager::ToggleMulticast()
{
    m_machineMgr->ToggleMulticast();
    SelectMachine();
}


void WindowManager::TagCurrent()
{
    m_machineMgr->TagCurrent();
}


void WindowManager::ForwardKeypress(int key)
{
    m_machineMgr->ForwardKeypress(key);
}


