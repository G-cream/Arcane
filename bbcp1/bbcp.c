#include "bbcp.h"

int 
main(int argc, char *argv[])
{
	char pathbuffer[PATH_MAX];
	char *buffer, *wcursor;
	char *source, *target;
	mode_t mode;
	int sfd, tfd;
	blksize_t buffermultiplier, rbuffersize, wbuffersize;
	ssize_t rnum;
	
	setprogname(argv[0]);
	source = argv[1];
	target = argv[2];
	/* illegal command */
	if (argc - 1 != ARG_SUPPORTED) 
		usage();
	/* make sure the path name is valid and the source is a file */
	if (!file_exists(source) || is_valid_dir(source))
		errx(EXIT_FAILURE, "Source doesn't exist or source is a directory!");
	if (is_valid_dir(target)) {
		cat_path_name(target, basename(source), pathbuffer);
		target = pathbuffer;
	}
	if (file_same(source, target)) {
		errx(EXIT_FAILURE, "Source and target are same files!");
	}
	sfd = open(source, O_RDONLY);
	if (sfd == -1)
		errx(EXIT_FAILURE, "Cannot open source!");
	tfd = open(target, O_WRONLY | O_CREAT | O_TRUNC);
	if (tfd == -1)
		errx(EXIT_FAILURE, "Cannot open or create target!");
	if (!get_file_ioblksize(source, &rbuffersize))
		errx(EXIT_FAILURE, "Cannot get read buffersize!");
	if (!get_file_ioblksize(target, &wbuffersize))
		errx(EXIT_FAILURE, "Cannot get write buffersize!");
	assert(rbuffersize != 0 && wbuffersize != 0);
	buffer = NULL;
	if (reallocarr(&buffer, rbuffersize, sizeof(*buffer)) != 0)
		errx(EXIT_FAILURE, "Cannot realloc io-buffersize of source!");
	while ((rnum = read(sfd, buffer, (size_t)rbuffersize)) != 0) {
		if (rnum == -1)
			errx(EXIT_FAILURE, "Read source error!");
		wcursor = buffer;
		for (buffermultiplier = rnum / wbuffersize; buffermultiplier != 0; --buffermultiplier) {				
			if (write(tfd, wcursor, wbuffersize) == -1)
				errx(EXIT_FAILURE, "Write target error!");
			wcursor += wbuffersize;
			rnum -= wbuffersize;
		}
		if (rnum != 0)
			if (write(tfd, wcursor, rnum) == -1)
				errx(EXIT_FAILURE, "Write target error!");
	}
	if (!get_file_mode(source, &mode))
		errx(EXIT_FAILURE, "Cannot get the mode of source!");
	if (fchmod(tfd, mode) == -1)
		errx(EXIT_FAILURE, "Cannot change the mode of target!");
	(void)close(sfd);
	(void)close(tfd);
	return EXIT_SUCCESS;
}

void
cat_path_name(char *prepath, char *sufpath, char *buffer)
{
	buffer[0] = '\0';
	(void)strcat(buffer, prepath);
	(void)strcat(buffer, "/");
	(void)strcat(buffer, sufpath);
}

void
usage(void)
{
	(void)fprintf(stderr, "usage: %s source_file target_file\n", getprogname());
	(void)fprintf(stderr, "usage: %s source_file target_directory\n", getprogname());
	exit(EXIT_FAILURE);
}