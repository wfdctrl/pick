#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

struct enode
{
	char *name;
	struct enode *next;
	mode_t mode;
	bool owns_name;
	bool root;
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
		ent->owns_name = false;
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

	while (ent != NULL)
	{
		puts(ent->name);
		ent = ent->next;
	} 
}

void print0_ent(struct enode *ents)
{
	struct enode *ent = ents;

	while (ent != NULL)
	{
		printf("%s", ent->name);
		putchar('\0');
		ent = ent->next;
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

void readdir_ent(struct enode *ent)
{
//	if (!S_ISDIR(ents->mode)) return;

	char *path = ent->name;
	int entc = 0;


	struct dirent *dirent;
	DIR *dir = opendir(path);

	while ((dirent = readdir(dir)) != NULL)
	{
		if (dirent->d_name[0] == '.') 
		{
			continue;
		}
		char *name = malloc(strlen(path) + strlen(dirent->d_name) + 3);
		strcpy(name, path);
		strcat(name, "/");
		strcat(name, dirent->d_name);

		if (entc == 0)
		{
			ent->name = name;
		}
		else
		{
			struct enode *ent_new = alloc_ent(1);
			ent_new->next = ent->next;
			ent->next = ent_new;
			ent_new->name = name;
			ent_new->owns_name = true;

			ent = ent_new;
		}
		if (dirent->d_type == DT_DIR)
		{
			readdir_ent(ent);
		}

		entc++;
	}
	closedir(dir);
}

void recurse_ent(struct enode *ents)
{
	struct enode *ent = ents;

	while (ent != NULL)
	{
		if(S_ISDIR(ent->mode))
		{
			readdir_ent(ent);
		}
		ent = ent->next;
	} 
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
	recurse_ent(ents);
	print_ent(ents);
	free_ent(ents);

	globfree(&globbuf);
}
