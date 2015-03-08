#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dirent.h>

struct enode
{
	const char *name;
	struct enode *next;
};

int main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "r0")) != -1)
	{
		switch (opt)
		{
			case 'r':
			break;
			case '0':
			break;
			default:
				abort();
				
		}
	}

	{
		int ind;

		glob_t globbuf;
		for (ind = optind; ind < argc; ind++) 
		{
			size_t globind;
			glob(argv[ind], GLOB_TILDE, NULL, &globbuf);
			for (globind = 0; globind < globbuf.gl_pathc; globind++)
			{
				puts(globbuf.gl_pathv[globind]);
			}
		}
		globfree(&globbuf);
	}
}
