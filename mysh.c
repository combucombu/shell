#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUFLEN 512
#define MAXCMDLEN 256
#define MAXSPLIT 80

int splitCmd(char* cmd, char* av[]);
int splitProc(char* av[], char* pav[], int pNum);
int countPipe(int ac, char* av[]);
int getCmd(char* cmd);
int sh_launch(char* av[], int pNum);
int removeRedirect(char* pav[], char* rav[]);
int main()
{
	int i;
	int k;
	char cmd[BUFLEN];
	char *av[MAXSPLIT];
	while (1) {
		int ac;
		int pNum;
		for (i = 0; i < BUFLEN; i++) {
			cmd[i] = '\0';
		}
		fprintf(stdout, "mysh$ ");
		getCmd(cmd);
		fprintf(stdout, "%s\n", cmd);
		ac = splitCmd(cmd, av);
		for (i = 0; i < ac; i++) {
			fprintf(stdout, "%d %s\n", i, av[i]);
		}
		if (strcmp(av[0], "exit") == 0) {
			exit(0);
		}
		fprintf(stdout, "\n");
		pNum = countPipe(ac, av) + 1;
		fprintf(stdout, "///////////////////////launch\n");
		sh_launch(av, pNum);	
		fprintf(stdout, "///////////////////////finish\n");
	}
}

int getCmd(char* cmd)
{
	int i;
	int k;
	char c;
	for (i = 0, k = 0; i < MAXCMDLEN; i++, k++) {
		if((c = fgetc(stdin)) == EOF) {
			fprintf(stderr, "Input error");
			exit(1);
		} else {
			if (c == '\0') {
				cmd[k] = c;
				return i;
			} else if (c == '\n') {
				cmd[k] = '\0';
				return i;
			} else if (c == '|') {
				cmd[k++] = ' ';
				cmd[k++] = '|';
				cmd[k] = ' ';
			} else if (c == '&') {
				cmd[k++] = ' ';
				cmd[k++] = '&';
				cmd[k] = ' ';
			} else {
				cmd[k] = c;
			}
		}
	}
	fprintf(stderr, "Command too long\n");
	exit(2);
}

int splitCmd(char* cmd, char* av[])
{
	int i = 0;
	int j = 0;
	av[0] = cmd;
	for (i = 1, j = 1; i < MAXCMDLEN; i++) {
		if (cmd[i] == '\0') {
			av[j++] = &cmd[i];
			break;
		}
		if (cmd[i] == ' ' || cmd[i] == '\t') {
			cmd[i] = '\0';
			if (cmd[i + 1] != ' ' && cmd[i + 1] != '\0' && cmd[i + 1] != '\t') {
				av[j++] = &cmd[i + 1];
			} 
		}
	}
	j--;
	return j;
}

int splitProc(char *av[], char *pav[], int pNum)
{
	int i;
	int j;
	int k;
	for (i = 0, j = 0; i < pNum; j++) {
		if (*av[j] == '|' || *av[j] == '\0') {
			i++;
		}
	}
	for (i = 0;; i++, j++) {
		if (*av[j] == '|' || *av[j] == '\0') {
			break;
		}
		pav[i] = av[j];
		fprintf(stderr, "pav[%d] = %s\n", i, pav[i]);
	}
	pav[i] = NULL;
	fprintf(stderr, "%s\n", pav[i]);

	fprintf(stderr, "\n");
	return i;
} 
int removeRedirect(char* pav[], char* rav[])
{
	int i;
	for (i = 0;;i++) {
		if(*pav[i] == '>') {
			rav[i] = NULL;
			break;
		}
		rav[i] = pav[i];
	}
	return i;
}


int countPipe(int ac, char *av[])
{
	int pipe = 0;
	int i;
	for (i = 0; i < ac; i++) {
		if (*av[i] == '|' || *av[i] == '&') {
			pipe++;
		}
	}
	return pipe;
}

int sh_launch(char* av[], int pNum)
{
	int i;
	int fd;
	int pac;
	int status;
	pid_t pid;
	char* pav[MAXSPLIT];
	char* rav[MAXSPLIT];

	fprintf(stdout, "pNum = %d\n", pNum);
	if (strcmp(av[0], "cd") == 0) {
		int return_code = 0;
		if (chdir(av[1]) == 0) {
			fprintf(stdout, "directory changed\n");
		} else {
			fprintf(stderr, "directory not changed !!!\n");
			return_code = 1;
		}
		return return_code;
	} else if (pNum == 1) {
		if ((pid = fork()) == 0) {
			pac = splitProc(av, pav, 0);
			for (i = 0; i < pac; i++) {
				if (*pav[i] == '>') {
					fd = open(pav[i + 1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
					close(1);
					dup(fd);
					close(fd);
					removeRedirect(pav, rav);
					execvp(rav[0], rav);
					break;
				}
				if (i == pac - 1) {	
					fprintf(stderr, "test\n");
					execvp(pav[0], pav);
					break;
				}
			}
			fprintf(stderr, "close\n");
		} else if (pid < 0){
			fprintf(stderr, "Error");
			exit(1);
		} else {
			wait(&status);
		}
	} else if (pNum == 2) {
		int pfd[2];
		pipe(pfd);

		if (fork() == 0) {
			fprintf(stderr, "fork1\n");
			close(1);
			dup(pfd[1]);
			close(pfd[0]);close(pfd[1]);
			pac = splitProc(av, pav, 0);
            for (i = 0; i < pac; i++) {
                if (*pav[i] == '>') {
                    fd = open(pav[i + 1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
                    close(1);
                    dup(fd);
                    close(fd);
                    removeRedirect(pav, rav);
                    execvp(rav[0], rav);
                    break;
                }
                if (i == pac - 1) {	
                    fprintf(stderr, "test\n");
                    execvp(pav[0], pav);
                    break;
                }
            }
			execvp(pav[0], pav);
		}
		if (fork() == 0) {
			fprintf(stderr, "fork2\n");
			close(0);
			dup(pfd[0]);
			close(pfd[0]);close(pfd[1]);
			pac = splitProc(av, pav, 1);
            for (i = 0; i < pac; i++) {
                if (*pav[i] == '>') {
                    fd = open(pav[i + 1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
                    close(1);
                    dup(fd);
                    close(fd);
                    removeRedirect(pav, rav);
                    execvp(rav[0], rav);
                    break;
                }
                if (i == pac - 1) {	
                    fprintf(stderr, "test\n");
                    execvp(pav[0], pav);
                    break;
                }
            }
			execvp(pav[0], pav);
		}
		close(pfd[0]);close(pfd[1]);
		wait(&status);wait(&status);
	} else if (pNum > 2) {
		int i;
		int pfd[pNum - 1][2];

		fprintf(stderr, "pNum = %d\n", pNum);
		//head process
		pipe(pfd[0]);
		if ((pid = fork()) == 0) {
			close(1);
			dup(pfd[0][1]);
			close(pfd[0][0]);
			close(pfd[0][1]);
			pac = splitProc(av, pav, 0);
			execvp(pav[0], pav);
		} else if (pid == -1) {
		} else {
			wait(&status);
		}
		//middle processes
		for (i = 1; i < pNum - 1; i++) {
			pipe(pfd[i]);
			if ((pid = fork()) == 0) {
				//close stdin & dup 
				close(0);
				dup(pfd[i - 1][0]);
				//close stdout & dup
				close(1);
				dup(pfd[i][1]);
				//close pipes
				close(pfd[i][0]);
				close(pfd[i][1]);
				close(pfd[i - 1][0]);
				close(pfd[i - 1][1]);
				pac = splitProc(av, pav, i);
				execv(pav[0], pav);
			} else if (pid == -1) {
			} else {
				close(pfd[i -1][0]);close(pfd[i - 1][1]);
				wait(&status);
			}
		}

		//tail process
		if ((pid = fork()) == 0) {
			close(0);
			dup(pfd[pNum - 2][0]);
			close(pfd[pNum - 2][0]);
			close(pfd[pNum - 2][1]);
			pac = splitProc(av, pav, pNum - 1);
			execvp(pav[0], pav);
		} else if (pid == -1) {
		} else {
			close(pfd[pNum - 2][0]); close(pfd[pNum - 2][1]);
			wait(&status);
		}
	}
	return 0;
}
