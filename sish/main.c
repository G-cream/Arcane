#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "sish.h"

void usage(void);
void run_onecommand();
void loop_command();

int
main(int argc, char **argv)
{	
	int opt;
	while ((opt = getopt(argc, argv, "xc:")) != -1) {
		switch (opt) {
		case 'c':
			CONFIG.cflag = true;
			if(!set_command(optarg))
				errx(EXIT_FAILURE, "Command format is invalid!\n");
			break;
		case 'x':
			CONFIG.xflag = true;
			break;
		case '?':
		default:
			usage();
			errx(EXIT_FAILURE, "argvs are invalid!\n");
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 0) {
		usage();
		errx(EXIT_FAILURE, "argvs are invalid!\n");
	}
	if (CONFIG.cflag)
		run_onecommand();
	else
		loop_command();
	return EXIT_SUCCESS;
}

void 
run_onecommand()
{
	int retcode;
	retcode = run_command();
	if (retcode == -1)
		errx(EXIT_FAILURE, "Inner error!\n");
	if (retcode == -2)
		warnx("Lexical syntax error!\n");
	if (retcode == -3)
		warnx("Syntactic syntax error!\n");
	//exit
	if(retcode == 1)
		return;
}

void 
loop_command()
{
	char commandtext[MAX_COMMANDTEXT_SIZE];	
	while (fgets(commandtext, MAX_COMMANDTEXT_SIZE, stdin) != NULL)
		run_onecommand();
	if (ferror(stdin))
		errx(EXIT_FAILURE, "Inputs command error!\n");
	return;
}

void
usage(void)
{
	(void)fprintf(stderr, "usage: sish [-x] [-c command]\n");
}