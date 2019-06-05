#include "sig_handle.h"
#include "csapp.h"
#include "eval.h"

/* sigint_handle - SIGINT handle */
void sigint_handle(int sig) {
    int olderrno = errno;
    int i;
    sigset_t mask_all, prev;

    Sigfillset(&mask_all);
    Sigprocmask(SIG_BLOCK, &mask_all, &prev);
    for (i = 0; i < MAXPROCS; i++)
        if (procs[i].state == FG) {
            killpg(procs[i].pid, SIGINT);
        }

    Sigprocmask(SIG_SETMASK, &prev, NULL);

    errno = olderrno;
    return;
}

/* sigtstp_handle SIGTSTP handle*/
void sigtstp_handle(int sig) {
    int olderrno = errno;
    int i;
    sigset_t mask_all, prev;

    Sigfillset(&mask_all);
    Sigprocmask(SIG_BLOCK, &mask_all, &prev);
    for (i = 0; i < MAXPROCS; i++)
        if (procs[i].state == FG) {
            killpg(procs[i].pid, SIGTSTP);
        }

    Sigprocmask(SIG_SETMASK, &prev, NULL);

    errno = olderrno;
    return;
}

/* sigchld_handle - SIGCHLD handle */
void sigchld_handle(int sig) {
    int olderrno = errno;

    if (stopped_resume_child > 0) {
        stopped_resume_child = 0;
        errno = olderrno;
        return;
    }

    sigset_t mask_all, prev;
    pid_t pid;
    int status;
    Sigfillset(&mask_all);
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        Sigprocmask(SIG_BLOCK, &mask_all, &prev);
        if (pid == fg_pid)
            fg_pid_reap = 1;
        struct proc_t *tmp_c = getproc_pid(procs, pid);
        if (WIFEXITED(status)) {
            // printf(" Job [%2d] %5d ended status: %d\n", tmp_c->jid, pid,
            //       WTERMSIG(status)); /* if bg process end print this message
            //       */
            delproclist(procs, pid);

        } else if (WIFSIGNALED(status)) {
            printf(" Job [%2d] %5d terminated by signal %d\n", tmp_c->jid, pid,
                   WTERMSIG(status));
            delproclist(procs, pid);
        } else if (WIFSTOPPED(status)) {
            tmp_c->state = ST;
            printf(" Job [%2d] %5d stopped by signal %d\n", tmp_c->jid, pid,
                   WSTOPSIG(status));
        }
        Sigprocmask(SIG_SETMASK, &prev, NULL);
    }

    errno = olderrno;
    return;
}