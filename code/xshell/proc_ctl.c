#include "csapp.h"
#include "eval.h"

volatile pid_t fg_pid = 0;               /* Foreground PID */
volatile pid_t stopped_resume_child = 0; /* Parent ignore SIGCONT */
volatile sig_atomic_t fg_pid_reap;       /* Parent can know child */

struct proc_t procs[MAXPROCS]; /* Processes list */

/* getproc_pid - get a process strcut from a pid */
struct proc_t *getproc_pid(struct proc_t *procs, pid_t pid) {
    int i;

    if (pid < 1)
        return NULL;

    for (i = 0; i < MAXPROCS; i++)
        if (procs[i].pid == pid)
            return &procs[i];

    return NULL;
}

/* getproc_jid - get a process strcut from a jid */
struct proc_t *getproc_jid(struct proc_t *procs, int jid) {
    int i;

    if (jid < 1)
        return NULL;

    for (i = 0; i < MAXPROCS; i++)
        if (procs[i].jid == jid)
            return &procs[i];

    return NULL;
}

/* addproclist - Add a job to the processes list */
int addproclist(struct proc_t *procs, pid_t pid, int state, char *cmdline) {
    int i;
    if (pid < 1)
        return 0;

    for (i = 0; i < MAXPROCS; i++) {
        if (procs[i].pid == 0) {
            procs[i].pid = pid;
            procs[i].jid = i + 1;
            procs[i].state = state;
            strcpy(procs[i].cmdline, cmdline);
            return 1;
        }
    }
    printf("Tried to create too many processes\n");
    return 0;
}

/* delproclist - Delete a job whose PID = pid frome the processes list */
int delproclist(struct proc_t *procs, pid_t pid) {
    int i, tmp_jid;
    if (pid < 1)
        return 0;

    for (i = 0; i < MAXPROCS; i++) {
        if (procs[i].pid == pid) {
            clearproclist(&procs[i]);
            return 1;
        }
    }
    return 0;
}

/* clearproclist - Clear a job struct  */
void clearproclist(struct proc_t *procs) {
    procs->jid = 0;
    procs->pid = 0;
    procs->state = UNDEF;
    procs->cmdline[0] = '\0';
}

/* initproclist - Initialize the job list */
void initproclist(struct proc_t *procs) {
    int i;

    for (i = 0; i < MAXPROCS; i++)
        clearproclist(&procs[i]);
}
