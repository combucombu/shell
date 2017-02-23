#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFLEN 512
#define MAXCMDLEN 256
#define MAXSPLIT 80

int splitCmd(char* cmd, char* av[]);
int splitProc(char* av[], char* pav[], int pNum);
int countPipe(int ac, char* av[]);
int getCmd(char* cmd);
int sh_launch(char* av[], int pNum);

int main()
{
	int i;
	int j, k;
	int pac;
	char cmd[BUFLEN];
	char *av[MAXSPLIT];
	char *pav[MAXSPLIT];
	while (1) {
		int ac;
		int pNum;
		//配列の初期化
		for (i = 0; i < MAXCMDLEN; i++) {
			cmd[i] = '\0';
		}
		//プロンプトの表示
		fprintf(stdout, "mysh$ ");
		//コマンドの取得
		getCmd(cmd);
		fprintf(stdout, "%s\n", cmd);
		ac = splitCmd(cmd, av);
		for (i = 0; i < ac; i++) {
			fprintf(stdout, "%d %s\n", i, av[i]);
		}
		fprintf(stdout, "\n");
		pNum = countPipe(ac, av) + 1;
		//コマンドの実行
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
	fprintf(stderr, "Command too long");
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
		fprintf(stdout, "pav[%d] = %s\n", i, pav[i]);
	}
	pav[i] = NULL;
	fprintf(stdout, "%s\n", pav[i]);

	fprintf(stdout, "\n");
	return i;
}

int countPipe(int ac, char *av[])
{
	int pipe = 0;
	int i;;
	for (i = 0; i < ac; i++) {
		if (*av[i] == '|' || *av[i] == '&') {
			pipe++;
		}
	}
	return pipe;
}

int sh_launch(char* av[], int pNum)
{
	int status;
	pid_t pid;
	char* pav[MAXSPLIT];
	pid = fork();
	
	if (pid == 0) {
		splitProc(av, pav, 0);
		execvp(pav[0], pav);
	} else if (pid != 1) {
		wait(&status);
	} else {
		fprintf(stderr, "Error");
		exit(1);
	}

	return 0;
}
