#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "log.h"
#include "config.h"
#include "window_manager.h"


static volatile int ZOMBIE_MACHINE_COUNT    = 0;
void SigchldHandler(int)
{
    ZOMBIE_MACHINE_COUNT++;
}


int main(int, char **)
{
    omnitty::OmniConfig::GetInstance()->LoadConfig();
    omnitty::InitLogger();

    LOG4CPLUS_INFO(omnitty::LOGGER_NAME, "Omnitty start running.");
    signal(SIGCHLD, SigchldHandler);

    omnitty::OmniWindowManager wndMgr;
    wndMgr.Init();
    
    pid_t chldpid;
    int ch = 0;
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

