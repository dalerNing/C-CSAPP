#include "eval.h"
#include "csapp.h"

void waitfg() {
    sigset_t mask_one, prev;
    Sigemptyset(&mask_one);
    // Sigaddset(&mask_one, SIGINT);
    // Sigaddset(&mask_one, SIGTSTP);
    Sigaddset(&mask_one, SIGCHLD);

    Sigprocmask(SIG_BLOCK, &mask_one, &prev);
    while (!fg_pid_reap)
        Sigsuspend(&prev);
    Sigprocmask(SIG_SETMASK, &prev, NULL);
}

/* eval - Evaluate a command line */
void eval(char *cmdline) {
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return;

    if (!builtin_command(argv)) {
        sigset_t mask_all, mask_one, prev;

        Sigfillset(&mask_all);
        Sigemptyset(&mask_one);
        // Sigaddset(&mask_one, SIGINT);
        // Sigaddset(&mask_one, SIGTSTP);
        Sigaddset(&mask_one, SIGCHLD);
        Sigprocmask(SIG_BLOCK, &mask_one, &prev); /* Block SIGCHID */

        if ((pid = Fork()) == 0) { /* Child runs user job */
            /* Child unblock SIGCHID */
            Sigprocmask(SIG_SETMASK, &prev, NULL);
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Commond not found.\n", argv[0]);
                exit(0);
            }
            Setpgid(0, 0); /* Creat a process group*/
        } else {           /* Parent waits for foreground job to terminate */
            if (!bg) {
                fg_pid = pid;
                fg_pid_reap = 0;
                /* Parent block all*/
                Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                /* Add the child to the process list */
                addproclist(procs, pid, FG, cmdline);
                /* Parent unblock SIGCHLD */
                Sigprocmask(SIG_SETMASK, &prev, NULL);
                /* Sigprocmask(SIG_BLOCK, &mask_one, &prev);
                while (!fg_pid_reap)
                    Sigsuspend(&prev);
                Sigprocmask(SIG_SETMASK, &prev, NULL); */
                waitfg();
            } else {
                /* Parent block all*/
                Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                /* Add the child to the process list */
                addproclist(procs, pid, BG, cmdline);
                /* Parent unblock SIGCHLD */
                Sigprocmask(SIG_SETMASK, &prev, NULL);

                printf("[%2d] %5d %s", getproc_pid(procs, pid)->jid, pid,
                       cmdline);
            }
        }
    }
    return;
}

/* If first arg is a builtin commond, run it and return true */
int builtin_command(char **argv) {

    if (!strcmp(argv[0], "quit")) /* quit command */
        exit(0);
    else if (!strcmp(argv[0], "&")) /* Ignore signleton & */
        return 1;
    else if (!strcmp(argv[0], "jobs")) {
        listjobs(procs);
        return 1;
    } else if (!strcmp(argv[0], "fg") || !strcmp(argv[0], "bg")) {
        do_fgbg(argv);
        return 1;
    }
    return 0; /* Not a builtin commond */
}

/* listjobs - Print the job list */
void listjobs(struct proc_t *procs) {
    int i;

    for (int i = 0; i < MAXPROCS; i++) {
        if (procs[i].pid != 0) {
            printf("[%2d] %5d ", procs[i].jid, procs[i].pid);
            switch (procs[i].state) {
            case FG:
                printf("Foreground ");
                break;
            case BG:
                printf("Running ");
                break;
            case ST:
                printf("Stopped ");
                break;
            default:
                printf("listjobs: Internal error: job[%d].state=%d ", i,
                       procs[i].state);
                break;
            }
            printf("%s", procs[i].cmdline);
        }
    }
}

/* do_fgbg */
void do_fgbg(char **argv) {
    char *first_arg = argv[0];

    if (!strcmp(first_arg, "fg")) {
        /* command fg, the process has two state:
         * the process running
         * the process stopped
         */
        if (argv[1] == NULL) {
            fprintf(stderr, "fg command need PID or %%JID argument\n");
            return;
        }
        pid_t pid;
        int jid;
        int state;
        struct proc_t *proc_tmp;

        if (argv[1][0] == '%') { /* JID */
            jid = atoi(argv[1] + 1);
            if (jid) {
                proc_tmp = getproc_jid(procs, jid);
                if (proc_tmp != NULL) {
                    state = proc_tmp->state;

                    fg_pid = proc_tmp->pid;
                    fg_pid_reap = 0;

                    proc_tmp->state = FG;
                    if (state == ST) {
                        stopped_resume_child = proc_tmp->pid;
                        kill(proc_tmp->pid, SIGCONT);
                    }
                    waitfg();
                } else
                    fprintf(stderr, "%%%s: No such job\n", argv[1] + 1);
            } else
                fprintf(stderr, "%%%s: No such job\n", argv[1] + 1);
        } else { /* PID */
            pid = atoi(argv[1]);
            if (pid) {
                proc_tmp = getproc_pid(procs, pid);
                if (proc_tmp != NULL) {
                    state = proc_tmp->state;

                    fg_pid = proc_tmp->pid;
                    fg_pid_reap = 0;

                    proc_tmp->state = FG;

                    if (state == ST) {
                        stopped_resume_child = proc_tmp->pid;
                        kill(proc_tmp->pid, SIGCONT);
                    }
                    waitfg();
                } else
                    fprintf(stderr, "%s: No such process\n", argv[1]);
            } else
                fprintf(stderr, "fg: argument must be a PID or %%JID\n");
        }
    } else {
        /* command bg */
        if (argv[1] == NULL) {
            fprintf(stderr, "bg command need PID or %%JID argument\n");
            return;
        }
        pid_t pid;
        int jid;
        struct proc_t *proc_tmp;

        if (argv[1][0] == '%') { /* JID */
            jid = atoi(argv[1] + 1);
            if (jid) {
                proc_tmp = getproc_jid(procs, jid);
                if (proc_tmp != NULL) {
                    stopped_resume_child = proc_tmp->pid;
                    proc_tmp->state = BG;
                    kill(proc_tmp->pid, SIGCONT);
                    printf("[%2d] %5d %s", proc_tmp->jid, proc_tmp->pid,
                           proc_tmp->cmdline);
                } else
                    fprintf(stderr, "%%%s: No such job\n", argv[1] + 1);
            } else
                fprintf(stderr, "%%%s: No such job\n", argv[1] + 1);
        } else { /* PID */
            pid = atoi(argv[1]);
            if (pid) {
                proc_tmp = getproc_pid(procs, pid);
                if (proc_tmp != NULL) {
                    stopped_resume_child = proc_tmp->pid;
                    proc_tmp->state = BG;
                    kill(proc_tmp->pid, SIGCONT);
                    printf("[%2d] %5d %s", proc_tmp->jid, proc_tmp->pid,
                           proc_tmp->cmdline);

                } else
                    fprintf(stderr, "%s: No such process\n", argv[1]);
            } else
                fprintf(stderr, "bg: argument must be a PID or %%JID\n");
        }
    }
}

/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) {
    char *delim; /* Points to first space delimiter */
    int argc;    /* Number of args */
    int bg;      /* Background job? */

    buf[strlen(buf) - 1] = ' ';   /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Bulid the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0) /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}
