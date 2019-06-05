#include "csapp.h"
#include "eval.h"
#include "sig_handle.h"

int main() {
    char cmdline[MAXLINE]; /* Command line */

    Signal(SIGINT, sigint_handle);   /* Ctrl+C */
    Signal(SIGTSTP, sigtstp_handle); /* Ctrl+Z */
    Signal(SIGCHLD, sigchld_handle); /* 回收僵尸子进程 */

    initproclist(procs);

    while (1) {
        /* Read */
        printf("> ");
        Fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    }
}
