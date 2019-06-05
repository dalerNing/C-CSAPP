#ifndef __SIG_HANDLE_H__
#define __SIG_HANDLE_H__

void sigint_handle(int sig);
void sigtstp_handle(int sig);
void sigchld_handle(int sig);

#endif