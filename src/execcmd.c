/*
 * Copyright 2005-2006 Vasil Dimov
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "execcmd.h"
#include "exhaust_fp.h"

#define PIPE_IN		1
#define PIPE_OUT	0

void
execcmd(const char *cmd, char *const args[],
	void (*process)(char *, void *), void *process_arg)
{
	int	p[2];
	pid_t	pid;
	int	status;

	FILE	*fp;

	if (pipe(p) == -1)
		err(EX_OSERR, "pipe()");

	switch ((pid = fork()))
	{
	case -1:
		err(EX_OSERR, "fork()");

		/* NOTREACHED */
		break;
	case 0:
		if (dup2(p[PIPE_IN], STDOUT_FILENO) == -1)
			err(EX_OSERR, "dup2()");

		execvp(cmd, args);

		/* FAILURE */

		err(EX_UNAVAILABLE, "execvp(): %s", cmd);

		/* NOTREACHED */
		break;
	default:
		if (close(p[PIPE_IN]) == -1)
			err(EX_OSERR, "close(): %d", p[PIPE_IN]);

		if ((fp = fdopen(p[PIPE_OUT], "r")) == NULL)
			err(EX_OSERR, "fdopen(): %d", p[PIPE_OUT]);

		exhaust_fp(fp, process, process_arg);

		if (fclose(fp) != 0)
			err(EX_OSERR, "fclose()");

		if (waitpid(pid, &status, 0) == -1)
			err(EX_OSERR, "waitpid()");

		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) != 0)
				errx(WEXITSTATUS(status),
				     "%s: exited with error %d",
				     cmd, WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			errx(EX_OSERR, "%s: exited on signal %d%s",
			     cmd, WTERMSIG(status),
			     WCOREDUMP(status) ? " (core dumped)" : "");
		}
	}
}

/* EOF */
