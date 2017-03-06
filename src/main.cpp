#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "log.h"
#include "config.h"
#include "window_manager.h"


static volatile int ZOMBIE_MACHINE_COUNT    = 0;
static const int LIST_WND_DEFAULT_CHARS     = 15;
static const int TERM_WND_DEFAULT_CHARS     = 80;
static const int TERM_WND_MIN               = 80;
static const std::string RTFM(
    "Syntax: omnitty [-W list_width] [-T term_width]\n\n"
    "     -W        specifies width of machine list area\n"
    "               (default is 8 characters)\n\n"
    "     -T        specifies width of terminal area\n"
    "               (default is 80 characters)\n\n");


void SigchldHandler(int)
{
    ZOMBIE_MACHINE_COUNT++;
}


int main(int argc, char **argv)
{
    omnitty::OmniConfig::GetInstance()->LoadConfig();
    omnitty::InitLogger();
    LOG4CPLUS_INFO(omnitty::LOGGER_NAME, "Omnitty start running.");

    int ch = 0;
    int listWndChars = LIST_WND_DEFAULT_CHARS;
    int termWndChars = TERM_WND_DEFAULT_CHARS;
    while (0 < (ch = getopt(argc, argv, "W:T:"))) {
        switch (ch) {
        case 'W':
            listWndChars = atoi(optarg);
            break;
        case 'T':
            termWndChars = atoi(optarg);
            if (termWndChars < TERM_WND_MIN) {
                LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "terminal area too narrow: %d", termWndChars);
                fputs(RTFM.c_str(), stderr);
                exit(2);
            }
            break;
        default:
            fputs(RTFM.c_str(), stderr);
            exit(2);
        }
    }

    signal(SIGCHLD, SigchldHandler);

    omnitty::OmniWindowManager wndMgr(listWndChars, termWndChars);
    wndMgr.Init();
    
    pid_t chldpid;
    bool quit = false;
    while (!quit) {
        if (ZOMBIE_MACHINE_COUNT) {
            --ZOMBIE_MACHINE_COUNT;
            chldpid = wait(NULL);
            wndMgr.HandleDeath(chldpid);
        }
        wndMgr.UpdateAllMachines();

        ch = getch();
        if (ch < 0) continue;
        wndMgr.Keypress(ch, ZOMBIE_MACHINE_COUNT);
    }
    
    omnitty::OmniConfig::GetInstance()->SaveConfig();
    endwin();
    return 0;
}

