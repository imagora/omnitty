#include "log.h"
#include "utils.h"
#include "config.h"
#include "curutil.h"
#include "opt_parser.h"
#include "machine_manager.h"
#include "window_manager.h"

using namespace omnitty;

/* minimum terminal dimensions to run program */
#define MIN_REQUIRED_WIDTH 80
#define MIN_REQUIRED_HEIGHT 25

static const std::string OMNITTY_VERSION("0.4.0");
static const std::string SPLASH_LINE_1("OmNiTTY Agora v" + OMNITTY_VERSION);
static const std::string SPLASH_LINE_2("Copyright (c) 2017 AgoraLab");
static const std::string REMINDER_LINE(
    "OmNiTTY-R v" + OMNITTY_VERSION +
    "  \007F1\007:menu"
    "  \006F2/3\007:sel"
    "  \003F4\007:tag"
    "  \002F5\007:add"
    "  \001F6\007:del"
    "  \005F7\007:mcast");

OmniWindowManager::OmniWindowManager()
    : m_machineMgr(std::make_shared<OmniMachineManager>()), m_menu(m_machineMgr),
      m_keypressFuncPtrs{
        {KEY_F(1), &OmniWindowManager::ShowMenu},
        {KEY_F(2), &OmniWindowManager::PrevMachine},
        {KEY_F(3), &OmniWindowManager::NextMachine},
        {KEY_F(4), &OmniWindowManager::TagCurrent},
        {KEY_F(5), &OmniWindowManager::AddMachine},
        {KEY_F(6), &OmniWindowManager::DeleteMachine},
        {KEY_F(7), &OmniWindowManager::ToggleMulticast},
    }
{
    m_listWndWidth = omnitty::OmniConfig::GetInstance()->GetListWndWidth();
    m_summaryWndWidth = omnitty::OmniConfig::GetInstance()->GetSummaryWndWidth();
    m_terminalWndWidth = omnitty::OmniConfig::GetInstance()->GetTerminalWndWidth();
}


OmniWindowManager::~OmniWindowManager()
{

}


void OmniWindowManager::Init()
{
    initscr();
    start_color();
    noecho();
    keypad(stdscr, TRUE);
    timeout(200);
    raw();
    CurutilColorpairInit();
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
    LOG4CPLUS_INFO_FMT(LOGGER_NAME, "window height: %d, width: %d", h, w);
    if (h < MIN_REQUIRED_HEIGHT || w < MIN_REQUIRED_WIDTH) {
        endwin();
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "omnitty requires a %d x %d terminal to run.",
            MIN_REQUIRED_WIDTH, MIN_REQUIRED_HEIGHT);
        exit(1);
    }

    wmove(stdscr, h / 2, (w - SPLASH_LINE_1.length())/2);
    CurutilAttrset(stdscr, 0x40);
    waddstr(stdscr, SPLASH_LINE_1.c_str());

    CurutilAttrset(stdscr, 0x70);
    wmove(stdscr, h/2 + 1, (w - SPLASH_LINE_2.length())/2);
    waddstr(stdscr, SPLASH_LINE_2.c_str());

    wrefresh(stdscr);
    while (getch() < 0 && i < 10) i++;

    wclear(stdscr);
    wrefresh(stdscr);

    DrawWindows();
}

void OmniWindowManager::LoadMachines()
{
    const std::string &machineFilePath = omnitty::OmniConfig::GetInstance()->GetMachineFilePath();
    if (!machineFilePath.empty()) {
        m_machineMgr->LoadMachines(machineFilePath);
    }
}


void OmniWindowManager::UpdateAllMachines()
{
    m_machineMgr->UpdateAllMachines();
    Redraw(false);
}


void OmniWindowManager::HandleDeath(pid_t pid)
{
    m_machineMgr->HandleDeath(pid);
}


void OmniWindowManager::Keypress(int key)
{
    auto iter = m_keypressFuncPtrs.find(key);
    if (iter == m_keypressFuncPtrs.end()) {
        ForwardKeypress(key);
        return;
    }
    (this->*(iter->second))();
}


void OmniWindowManager::DrawWindows()
{
    /* obtain terminal dimensions */
    int totalWidth = 0;
    int totalHeight = 0;
    getmaxyx(stdscr, totalHeight, totalWidth);
    LOG4CPLUS_INFO_FMT(omnitty::LOGGER_NAME, "get the window size, width: %d height: %d", totalWidth, totalHeight);

    // reset the size
    m_terminalWndWidth = totalWidth - (m_listWndWidth + m_summaryWndWidth + 2);

    /* the geometry is hard-coded here, but nowhere else... so I don't
     * see a lot of point using #defines or anything any more sophisticated */
    int A = m_listWndWidth + 2;
    int C = totalWidth - m_terminalWndWidth;
    int B = C - 1;
    if (B < A) {
        B = A;
        C = B + 1;
    }

    int vtrows = totalHeight - 3;
    int vtcols = totalWidth - C;

    /* actually create the windows */
    m_listWnd = newwin(totalHeight - 3, A - 0, 1, 0);
    m_summaryWnd = (B - A >= 3) ? newwin(totalHeight-3, B - A, 1, A) : nullptr;
    m_virtualTerminalWnd = newwin(totalHeight-3, vtcols, 1, C);
    m_menu.InitMenu(totalWidth, totalHeight-1);

    /* draw the top decoration line */
    wattrset(stdscr, COLOR_PAIR(3) | A_BOLD);
    wmove(stdscr, 0, 0);
    whline(stdscr, ACS_HLINE | A_NORMAL, totalWidth);

    /* draw instruction line */
    wattrset(stdscr, COLOR_PAIR(4 * 8) | A_BOLD);
    wmove(stdscr, totalHeight-2, 0);
    whline(stdscr, ' ', totalWidth);
    wmove(stdscr, totalHeight-2, 0);
    const char *p = REMINDER_LINE.c_str();
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
    wvline(stdscr, ACS_VLINE | A_NORMAL, totalHeight - 2);
    wmove(stdscr, 0, B);
    waddch(stdscr, ACS_TTEE);
    wrefresh(stdscr);

    /* draw window titles */
    if (totalWidth > 90) {
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


void OmniWindowManager::DrawMachineList()
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

        CurutilAttrset(m_listWnd, attr);
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


void OmniWindowManager::DrawSummary()
{
    int sumheight, sumwidth;
    werase(m_summaryWnd);
    wmove(m_summaryWnd, 0, 0);
    getmaxyx(m_summaryWnd, sumheight, sumwidth);

    uint32_t scrollPos = static_cast<uint32_t>(m_machineMgr->GetScrollPos());
    uint32_t machineCount = m_machineMgr->GetMachineCount();
    for (uint32_t i = scrollPos; i < scrollPos + sumheight && i < machineCount; ++i) {
        CurutilAttrset(m_summaryWnd, 0x80);
        wmove(m_summaryWnd, i - scrollPos, 0);
        std::string summary(m_machineMgr->MakeVirtualTerminalSummary(i, sumwidth));
        waddstr(m_summaryWnd, summary.c_str());
    }
}


void OmniWindowManager::DrawVirtualTerminal()
{
    werase(m_virtualTerminalWnd);
    int selectedMachine = m_machineMgr->GetSelectedMachine();
    if (selectedMachine >= 0 && selectedMachine < static_cast<int>(m_machineMgr->GetMachineCount())) {
        MachinePtr machine = m_machineMgr->GetMachine(selectedMachine);
        rote_vt_draw(machine->GetVirtualTerminal(), m_virtualTerminalWnd, 0, 0, NULL);
    }
}


void OmniWindowManager::Redraw(bool forceFullRedraw)
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


void OmniWindowManager::ShowMenu()
{
    m_menu.ShowMenu();
    SelectMachine();
    Redraw(true);
}


void OmniWindowManager::PrevMachine()
{
    m_machineMgr->PrevMachine();
    SelectMachine();
}


void OmniWindowManager::NextMachine()
{
    m_machineMgr->NextMachine();
    SelectMachine();
}


void OmniWindowManager::TagCurrent()
{
    m_machineMgr->TagCurrent();
}


void OmniWindowManager::AddMachine()
{
//    std::vector<char *> buf2;
//    if (m_menu.Prompt("Add(ip/f file/g group): ", 0xE0, buf2, 256, 256)) {
//        LOG4CPLUS_INFO_FMT(omnitty::LOGGER_NAME, "111111: %s", buf2[0]);
//        LOG4CPLUS_INFO_FMT(omnitty::LOGGER_NAME, "222222: %s", buf2[1]);
//    }
    std::string buf(256, '\0');
    if (m_menu.Prompt("Add(ip/f file/g group): ", 0xE0, &buf[0], 256)) {
        StripString(buf);

        std::string machineFile;
        std::string machineGroup;
        IpV4Pair ipPair;
        OmniOptParser optParser;
        optParser.AddLongOpt("f", &machineFile);
        optParser.AddLongOpt<std::string>("g", &machineGroup);
        optParser.AddLongOpt<IpV4Pair>("mtr", &ipPair);
        optParser.AddLongOpt<IpV4Pair>("traceroute", &ipPair);

//        if (!optParser.ParseOpts(3, buf.c_str())) {
//            //
//        }

        std::vector<std::string> params(SplitString(buf, ' '));
        if (params.size() == 1) {
            m_machineMgr->AddMachine(OmniConfig::GetInstance()->GetCommand(buf));
        } else if (params.size() == 2) {
            if (params[0] == "f") {
                LOG4CPLUS_INFO_FMT(omnitty::LOGGER_NAME, "add machine from file: %s", params[1].c_str());
                AddMachinesFromFile(params[1]);
            } else if (params[0] == "g") {
                LOG4CPLUS_INFO_FMT(omnitty::LOGGER_NAME, "add machine from group: %s", params[1].c_str());
                AddMachinesFromGroup(params[1]);
            } else {
                LOG4CPLUS_WARN_FMT(omnitty::LOGGER_NAME, "cannot parse machine param info: %s", buf.c_str());
                return;
            }
        } else {
            LOG4CPLUS_WARN_FMT(omnitty::LOGGER_NAME, "cannot parse machine info: %s, param size: %u", buf.c_str(), static_cast<uint32_t>(params.size()));
            return;
        }
    }
    SelectMachine();
}


void OmniWindowManager::AddMachinesFromFile(const std::string &file)
{
    if (!m_machineMgr->LoadMachines(file)) {
        m_menu.ShowMessageAndWait("Can't read that file.", 0xF1);
        return;
    }

    m_menu.ShowMessageNotWait("Adding machines from file, and load group default...", 0x70);
    AddMachinesFromGroup("default");
}


void OmniWindowManager::AddMachinesFromGroup(const std::string &group)
{
    m_machineMgr->AddMachinesFromGroup(group);
}


void OmniWindowManager::DeleteMachine()
{
    static char buf[2] = {0};

    if (m_menu.Prompt("Really delete it [y/n]?", 0x90, buf, 2) && (*buf == 'y' || *buf == 'Y')) {
        m_machineMgr->DeleteCurrentMachine();
    }
    SelectMachine();
}


void OmniWindowManager::ToggleMulticast()
{
    m_machineMgr->ToggleMulticast();
    SelectMachine();
}


void OmniWindowManager::ForwardKeypress(int key)
{
    m_machineMgr->ForwardKeypress(key);
}


void OmniWindowManager::SelectMachine()
{
    int screenwidth, screenheight;
    getmaxyx(m_listWnd, screenheight, screenwidth);
    m_machineMgr->ResetSelectedMachine(screenheight);
}

