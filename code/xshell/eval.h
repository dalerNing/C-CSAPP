#ifndef __EVAL_H__
#define __EVAL_H__

#include "csapp.h"

/* Misc manifest constants */
#define MAXARGS 128 /* max args on a command line */
#define MAXPROCS 10 /* max processes at any point in time */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

extern volatile pid_t fg_pid;
extern volatile pid_t stopped_resume_child;
extern volatile sig_atomic_t fg_pid_reap;

struct proc_t {            /* processes struct */
    int jid;               /* job ID */
    pid_t pid;             /* process ID */
    int state;             /* UNDEF, FG, BG, ST */
    char cmdline[MAXARGS]; /* command line */
};

extern struct proc_t procs[MAXPROCS]; /* Processes list */

void waitfg();

void eval(char *cmdline);
int builtin_command(char **argv);

void listjobs(struct proc_t *procs);
void do_fgbg(char **argv);

int parseline(char *buf, char **argv);

/*
 ***** Processes control *****
 */
struct proc_t *getproc_pid(struct proc_t *procs, pid_t pid);
struct proc_t *getproc_jid(struct proc_t *procs, int pid);

int addproclist(struct proc_t *procs, pid_t pid, int state, char *cmdline);
int delproclist(struct proc_t *procs, pid_t pid);
void clearproclist(struct proc_t *procs);
void initproclist(struct proc_t *procs);

#endif