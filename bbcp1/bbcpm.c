#include "bbcpm.h"

int 
main(int argc, char *argv[])
{
	char pathbuffer[PATH_MAX];
	char *source, *target;
	mode_t mode;
	int sfd, tfd;
	void *saddr, *taddr;
	struct stat	st;
	off_t cpsize;
	size_t buffersize;
	
	setprogname(argv[0]);	
	source = argv[1];
	target = argv[2];
	cpsize = 0;
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
	if (fstat(sfd, &st) < 0)
		errx(EXIT_FAILURE, "Cannot get the stat of source!");
	if (ftruncate(tfd, st.st_size) < 0)
		errx(EXIT_FAILURE, "Cannot truncate the target!");	
	while (cpsize != st.st_size) {
		if (st.st_size >= MAX_CP_SIZE)
			buffersize = MAX_CP_SIZE;
		else
			buffersize = st.st_size - cpsize;
		saddr = mmap(0, buffersize, PROT_READ, MAP_SHARED, sfd, cpsize);
		if(saddr == MAP_FAILED)
			errx(EXIT_FAILURE, "Cannot map the source!");	
		taddr = mmap(0, buffersize, PROT_WRITE, MAP_SHARED, tfd, cpsize);
		if (taddr == MAP_FAILED)
			errx(EXIT_FAILURE, "Cannot map the target!");	
		memcpy(taddr, saddr, buffersize);
		(void)munmap(saddr, buffersize);
		(void)munmap(taddr, buffersize);
		cpsize += buffersize;
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