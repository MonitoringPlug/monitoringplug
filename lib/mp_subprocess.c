/***
 * Monitoring Plugin - mp_subprocess.c
 **
 *
 * Copyright (C) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * $Id$
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "mp_subprocess.h"
#include "mp_common.h"

sig_t mp_subprocess_alarm;

void subprocess_timeout_alarm_handler(int signo);

mp_subprocess_t *mp_subprocess(char *command[]) {
    int             pfp[2];
    char            *env[2] = {"LC_ALL=C", NULL};
    struct stat     fileStat;
    mp_subprocess_t *sph;

    // Check for execute bit
    if(access(command[0], X_OK) != 0) {
        if (mp_verbose > 0)
            fprintf(stderr, "Can open '%s' for executing:\n%s",
                    command[0], strerror(errno));
        return NULL;
    }

    // Check for regular file mode.
    if (stat(command[0], &fileStat) < 0) {
        if (mp_verbose > 0)
            fprintf(stderr, "stat '%s' failed:\n%s",
                    command[0], strerror(errno));
        return NULL;
    }
    if (!S_ISREG(fileStat.st_mode)) {
        if (mp_verbose > 0)
            fprintf(stderr, "'%s'  is no regular file.\n", command[0]);
        return NULL;
    }

    /* Create pipe */
    if (pipe(pfp) == -1) {
        if (mp_verbose > 0)
            perror("Creating pipes failed.");
        return NULL;
    }

    /* Init suprocess handle. */
    sph = mp_malloc(sizeof(mp_subprocess_t));
    memset(sph, 0, sizeof(mp_subprocess_t));
    sph->sp_stdin = pfp[1];
    sph->sp_stdout = pfp[0];

    /* Fork subprocess way */
    sph->pid = fork();
    if (sph->pid == -1) {
        close(pfp[0]);
        close(pfp[1]);
        perror("Fork failed.");
        return NULL;
    }

    /* Parent */
    if (sph->pid > 0) {
        mp_subprocess_alarm = (sig_t)signal(SIGALRM, subprocess_timeout_alarm_handler);

        return sph;
    }

    /* Child */
    if (dup2(pfp[0], 0) == -1)
        exit(1);
    if (close(pfp[0]) == -1)
        exit(1);

    if (dup2(pfp[1], 1) == -1)
        exit(1);
    if (close(pfp[1]) == -1)
        exit(1);

    execve(command[0], command, env);

    exit(1);
}

int mp_subprocess_close(mp_subprocess_t *subprocess) {
    int status;

    /* Reset alarm handler. */
    signal(SIGALRM, mp_subprocess_alarm);

    while (waitpid(subprocess->pid, &status, 0) < 0 ){
        if (errno != EINTR)
            return 1;
    }
    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return -1;
}

void subprocess_timeout_alarm_handler(int signo) {
    if (signo == SIGALRM) {
        critical("Plugin timed out in subprocess after %d seconds\n", mp_timeout);
    }
}

/* vim: set ts=4 sw=4 et syn=c : */
