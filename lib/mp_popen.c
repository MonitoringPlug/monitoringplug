/***
 * monitoringplug - mp_popen.c
 **
 *
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "mp_popen.h"
#include "mp_common.h"

pid_t *mp_childpid;
sig_t *mp_popen_alarm;

void popen_timeout_alarm_handler(int signo);

FILE *mp_popen(char *command[]) {

    int pfp[2];
    char *env[2] = {"LC_ALL=C", NULL};
    pid_t pid;
    struct stat fileStat;


    if(access(command[0], R_OK | X_OK) != 0)
        return NULL;

    if (stat(command[0], &fileStat) < 0)
        return NULL;
    //if(((fileStat->st_mode) & S_IFMT) != S_IFREG)
    if (!S_ISREG(fileStat.st_mode))
        return NULL;

    // Create pipe
    if (pipe(pfp) == -1)
        return NULL;

    // Fork away
    pid = fork();
    if (pid == -1) {
        close(pfp[0]);
        close(pfp[1]);
        return NULL;
    }

    // Parent
    if (pid > 0) {
        if (close(pfp[1]) == -1)
            return NULL;
        if (mp_childpid == NULL) {
            if ((mp_childpid = calloc((size_t) getdtablesize(), sizeof(pid_t)))
                    == NULL)
                return NULL;
        }
        mp_childpid[pfp[0]] = pid;

        mp_popen_alarm = (sig_t)signal(SIGALRM, popen_timeout_alarm_handler);

        return fdopen(pfp[0], "r");
    }

    // Child
    if (close(pfp[0]) == -1)
        exit(1);

    if (dup2(pfp[1], 1) == -1) {
        exit(1);
    }

    if (close(pfp[1]) == -1)
        exit(1);

    execve(command[0], command, env);

    exit(1);
}

int mp_pclose(FILE * file) {
    int status;
    pid_t pid;
    pid = mp_childpid[fileno(file)];
    mp_childpid[fileno(file)] = 0;

    if (pid == 0)
        return pclose(file);

    fclose(file);

    signal(SIGALRM, mp_popen_alarm);

    while (waitpid(pid, &status, 0) < 0)
        if (errno != EINTR)
            return (1);

    if (WIFEXITED (status))
        return (WEXITSTATUS (status));

    return (1);

}

void popen_timeout_alarm_handler(int signo) {
    if (signo == SIGALRM) {
        critical("Plugin timed out in popen after %d seconds\n", mp_timeout);
    }
}

