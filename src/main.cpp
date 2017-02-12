#include <ncurses.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "curutil.h"
#include "machine.h"
#include "window_manager.h"
#include "menu.h"


/* how many characters wide the list window will be, by default */
#define LISTWIN_DEFAULT_CHARS 8
#define TERMWIN_DEFAULT_CHARS 80
#define TERMWIN_MIN 80


#define RTFM "Syntax: omnitty [-W list_width] [-T term_width]\n" \
"\n" \
"     -W        specifies width of machine list area\n" \
"               (default is 8 characters)\n" \
"\n" \
"     -T        specifies width of terminal area\n" \
"               (default is 80 characters)\n" \
"\n"


static volatile int zombie_count = 0;
void sigchld_handler(int signo) {
    (void)signo;
    zombie_count++;
}


int main(int argc, char **argv) {
    int ch = 0;
    int list_win_chars = LISTWIN_DEFAULT_CHARS;
    int term_win_chars = TERMWIN_DEFAULT_CHARS;
    /* process command-line options */
    while (0 < (ch = getopt(argc, argv, "W:T:"))) {
        switch (ch) {
        case 'W':
            list_win_chars = atoi(optarg);
            break;
        case 'T':
            term_win_chars = atoi(optarg);
            if( term_win_chars < TERMWIN_MIN ) {
                fprintf(stderr, " terminal area too narrow: %d\n", term_win_chars);
                fputs(RTFM, stderr);
                exit(2);
            }
            break;
        default:
            fputs(RTFM, stderr);
            exit(2);
        }
    }

    signal(SIGCHLD, sigchld_handler);

    WindowManager wndMgr(list_win_chars, term_win_chars);
    wndMgr.InitCurses();
    wndMgr.InitWindows();
    
    pid_t chldpid;
    bool quit = false;
    while (!quit) {
        if (zombie_count) {
            zombie_count--;
            chldpid = wait(NULL);
            wndMgr.HandleDeath(chldpid);
        }
        
        wndMgr.UpdateAllMachines();
        wndMgr.Redraw(false);
        ch = getch();
        if (ch < 0) continue;
        
        switch (ch) {
        case KEY_F(1):
            wndMgr.ShowMenu();
            wndMgr.Redraw(true);
            break;
        case KEY_F(2):
            wndMgr.PrevMachine();
            break;
        case KEY_F(3):
            wndMgr.NextMachine();
            break;
        case KEY_F(4):
            wndMgr.TagCurrent();
            break;
        case KEY_F(5):
            wndMgr.AddMachine(zombie_count);
            break;
        case KEY_F(6):
            wndMgr.DeleteMachine();
            wndMgr.SelectMachine();
            break;
        case KEY_F(7):
            wndMgr.ToggleMulticast();
            break;
        default:
            wndMgr.ForwardKeypress(ch);
            break;
        }
    }
    
    endwin();
    return 0;
}

