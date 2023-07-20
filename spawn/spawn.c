/*  Make the necessary includes and set up the variables.  */


#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include<sys/wait.h>
#include <grp.h>
#include <pwd.h>


static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
}


static int spawn(const char *appPath, char **appArgv,const char *user,const char *group) {
	pid_t child;
	int i =0 ;
	child = fork();					
	switch (child) {
		case 0: { //child
			
			int max_fd = 0;		
			/* loose control terminal */
			setsid();
			if ( getuid() == 0) {//root   
				struct group *grp = NULL;
				struct passwd *pwd = NULL;
				if (*user) {
					if (NULL == (pwd = getpwnam(user))) {
						fprintf(stderr, "[supervise]can't find username: %s \n",
							 user);
						exit(-1);
					}
				}
				if (*group) {
					if (NULL == (grp = getgrnam(group))) {
						fprintf(stderr, "[supervise]can't find groupname: %s\n",
							 group);
						exit(1);
					}
					/* do the change before we do the chroot() */
					setgid(grp->gr_gid);
					setgroups(0, NULL);
					if (*user) {
						initgroups(user, grp->gr_gid);
					}
				}
				/* drop root privs */
				if (*user) {
					setuid(pwd->pw_uid);
				}
			}
			max_fd = open("/dev/null", O_RDWR);
			
			if (-1 != max_fd) {
				if (max_fd != STDOUT_FILENO) dup2(max_fd, STDOUT_FILENO);
				if (max_fd != STDERR_FILENO) dup2(max_fd, STDERR_FILENO);
				if (max_fd != STDOUT_FILENO && max_fd != STDERR_FILENO) close(max_fd);
			} else {
				fprintf(stderr, "spawn-fcgi: couldn't open and redirect stdout/stderr to '/dev/null': %s\n", strerror(errno));
			}
			
			for (i = 3; i < max_fd; i++)  close(i);
			
			char *b = (char*)malloc(strlen("exec ") + strlen(appPath) + 1);
			strcpy(b, "exec ");
			strcat(b, appPath);
			/* exec the app */
			execl("/bin/sh", "sh", "-c", b, (char *)NULL);
			/* in nofork mode stderr is still open */
			fprintf(stderr, "spawn: exec failed: %s\n", strerror(errno));
			exit(errno);
			break;
		}
		case -1:
			/* error */
			fprintf(stderr, "spawn: fork failed: %s\n", strerror(errno));
			break;
		default:
			/* father */
			break;
		}
	return child;
}

static int show_help () {
	const char *b = \
"Usage: spawn [options] [-- <app> [app arguments]]\n" \
"\n" \
"Options:\n" \
" -f <path>     filename of the application \n" \
" -d            run at daemon mode\n"\
" -u user       start processes using specified linux user\n" \
" -g group      start processes using specified linux group\n"
;
	return write(1, b, strlen(b));
}

int main(int argc, char **argv)
{
	signed int i;
	int daemon_mode = 0;
	const char *app = NULL;
	const char *user= "work";
	const char *group= "work";
	pid_t child = 0;
	
	if (argc < 2) { /* no arguments given */
		show_help();
		return -1;
	}
	while (-1 != (i = getopt(argc, argv, "f:u:g:hd"))) {
		switch(i) {
		case 'f': app = optarg; break;
		case 'd': daemon_mode = 1; break;
		case 'u': user  = optarg; break;
		case 'g': group = optarg; break;
		case 'h': show_help(); return 0;
		default:
			show_help();
			return -1;
		}
	}
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	if(daemon_mode) daemon(1, 1);//1:no change dir  0:output to /dev/null
	
	while (!got_sig_term) {
		do {
			if (child) {
				int status;
				int r = waitpid(child, &status, WNOHANG);//WNOHANG  子进程没有结束则返回0
				if (r == child) {
					if (WIFEXITED(status)) {
						fprintf(stderr, "spawn: child exited with: %d\n",
						WEXITSTATUS(status));
					} else if (WIFSIGNALED(status)) {
						fprintf(stderr, "spawn: child signaled: %d\n",
						WTERMSIG(status));
					} else {
						fprintf(stderr, "spawn: child died somehow: exit status = %d\n",
						status);
					}
				} else {
					if(!daemon_mode) {fprintf(stdout, "spawn: child spawned successfully: PID: %d\n", child); return 0;}
					break;
				}
			}
			child =spawn(app,NULL,user,group); 
		} while (0);
		sleep(5);
	}
	return 0;
}

