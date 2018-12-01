#include <err.h>
#include "sws.h"

int
main(int argc, char **argv)
{
	struct swsstate sws;	
	int opt;

	(void)init_sws(&sws);
	while ((opt = getopt(argc, argv, "c:dhi:l:p:")) != -1) {
		switch (opt) {
		case 'c':
			if (!set_sws_cgidir(&sws, optarg))
				errx(EXIT_FAILURE, "CGI dir is invalid!\n");
			sws.cgiflag = true;
			break;
		case 'd':
			sws.debuggingflag = true;
			break;
		case 'h':
			sws.usageflag = true;
			break;
		case 'i':
			sws.ipflag = true;
			if (!set_sws_ipaddress(&sws, optarg))
				errx(EXIT_FAILURE, "IP address is invalid!\n");
			break;
		case 'l':
			sws.logflag = true;
			break;
		case 'p':
			sws.portflag = true;
			if (!set_sws_portnumber(&sws, optarg))
				errx(EXIT_FAILURE, "Port number is invalid!\n");
			break;
		case '?':
		default:
			usage(STDERR_FILENO);
			exit(EXIT_FAILURE);
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;
	
	if (argc == 0)
		errx(EXIT_FAILURE, "Dir is missing!\n");
	if (argc != 1 || !set_sws_rootdir(&sws, argv[0]))
		errx(EXIT_FAILURE, "Root dir is invalid!\n");
	int result = process_sws(&sws);
	switch (result) {
	case -1:
		/*TODO
		 *
		 *
		 **/
		exit(EXIT_FAILURE);
	default:
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}	
	
	return EXIT_SUCCESS;
}