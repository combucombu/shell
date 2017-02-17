#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAXCMDLEN 256

int main()
{
	int i;
	char c[MAXCMDLEN];
	char dbg;

	while (1) {
		for (i = 0; i < MAXCMDLEN; i++) {
			c[i] = '\0';
		}
		fprintf(stdout, "mysh$ ");
		if (fgets(c, MAXCMDLEN, stdin) == NULL) {
			fprintf(stderr, "Input error");
			exit(1);
		}
		if (c[MAXCMDLEN	- 1] != '\0') {
			fprintf(stdout, "String too long\n");
			continue;
		}
		
		fprintf(stdout, "%s", c);
	}
}
