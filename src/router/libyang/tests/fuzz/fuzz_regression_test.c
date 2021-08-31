#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include<fcntl.h>

int main(int argc, char **argv) 
{
	DIR *d;
	struct dirent *dir;
	pid_t p = 0;
	int input_fd = 0;
	int status = 0;
	int rc = 0;
	struct stat path_stat;

	if (argc != 3) {
		fprintf(stderr, "invalid number of arguments. Call like this ./fuzz_regression_test fuzz_harness corpus_dir\n");
		return EXIT_FAILURE;
	}

	d = opendir(argv[2]);
	if (!d) {
		fprintf(stderr, "error opening dir %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	while ((dir = readdir(d)) != NULL) {
		stat(dir->d_name, &path_stat);
		if (!S_ISREG(path_stat.st_mode)) {
			continue;
		}

		p = fork();
		if (p == -1) {
			fprintf(stderr, "fork failed\n");
			return EXIT_FAILURE;
		} else if (p == 0) {
			input_fd = open(dir->d_name, O_RDONLY);
			if (input_fd == -1) {
				fprintf(stderr, "error opening input file %s\n", dir->d_name);
				return EXIT_FAILURE;
			}
			
			dup2(input_fd, STDIN_FILENO);
			execl(argv[1], argv[1], NULL);
			return EXIT_SUCCESS;
		}

		rc = waitpid(p, &status, 0);
		if (rc == -1) {
			fprintf(stderr, "waitpid failed\n");
			return EXIT_FAILURE;
		}

		if (!WIFEXITED(status)) {
			fprintf(stderr, "test %s - %s failed\n", argv[1], dir->d_name);
			return EXIT_FAILURE;
		}

		printf("test %s - %s successful\n", argv[1], dir->d_name);
	}

	closedir(d);

	return EXIT_SUCCESS;
}
