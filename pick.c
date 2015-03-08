#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

struct enode
{
	char *name;
	struct enode *next;
	mode_t mode;
};

int count_ent(struct enode *ents)
{
	struct enode *ent = ents;
	int count = 0;

	while ((ent = ent->next) != NULL)
	{
		count++;
	}

	return count;
}

struct enode *alloc_ent(int count)
{
	return malloc(count * sizeof(struct enode));
}

struct enode *realloc_ent(struct enode *ents, int count)
{
	if (count_ent(ents) < count)
	{
		return realloc(ents, count * sizeof(struct enode));
	}
	else
	{
		return ents;
	}
}

void free_ent(struct enode *ents)
{
	free(ents);
}

void create_ent(struct enode *ents, int count, char *values[])
{
	int ind;
	struct stat sb;

	for (ind = 0; ind < count; ind++)
	{
		struct enode *ent = &ents[ind];
		ent->name = values[ind];
		stat(ent->name, &sb);
		ent->mode = sb.st_mode;

		if (ind != 0)
		{
			ents[ind - 1].next = ent;
		}
	}
}

void print_ent(struct enode *ents)
{
	struct enode *ent = ents;

	while ((ent = ent->next) != NULL)
	{
		puts(ent->name);
	}
}

void print0_ent(struct enode *ents)
{
	struct enode *ent = ents;

	while ((ent = ent->next) != NULL)
	{
		printf("%s", ent->name);
		putchar('\0');
	}
}

void glob_ent(struct enode *ents, glob_t *globbuf)
{
	struct enode *ent = ents;

	glob(ent->name, GLOB_TILDE, NULL, globbuf);
	while ((ent = ent->next) != NULL)
	{
		glob(ent->name, GLOB_TILDE | GLOB_APPEND, NULL, globbuf);
	}
	realloc_ent(ents, globbuf->gl_pathc);
	create_ent(ents, globbuf->gl_pathc, globbuf->gl_pathv);
}

struct enode *recurse_ent(struct enode *ents)
{

}

int main(int argc, char *argv[])
{
	int opt;
	glob_t globbuf;
	struct enode *ents;

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

	ents = alloc_ent(argc - optind);
	create_ent(ents, argc - optind, argv + optind);
	glob_ent(ents, &globbuf);
	print_ent(ents);
	free_ent(ents);

	globfree(&globbuf);
}
