#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

enum options 
{
	OPT_DIR = 0x1,
	OPT_RECURSIVE = 0x2,
	OPT_NULL = 0x4
};
int options = 0;

struct enode
{
	char *name;
	mode_t mode;
	bool owns_name;
};

struct ent_vec
{
	struct enode *data;
	size_t capacity;
	size_t size;
};

void ent_init(struct ent_vec *self, size_t size)
{
	self->data = malloc(size * sizeof(struct enode));
	self->capacity = size;
	self->size = 0;
}

void ent_resize(struct ent_vec *self, size_t size)
{
	if (self->capacity < size)
	{
		self->data = realloc(self->data, size * sizeof(struct enode));
		self->capacity = size;
	}
}

void ent_free(struct ent_vec *self)
{
	free(self->data);
}

struct enode *ent_beg(struct ent_vec *self)
{
	return &self->data[0];
}

struct enode *ent_end(struct ent_vec *self)
{
	return &self->data[self->size];
}

struct enode *ent_at(struct ent_vec *self, size_t ind)
{
	return &self->data[ind];
}

void ent_add(struct ent_vec *self, char *name, bool owns_name)
{
	struct stat sb;

	if (self->capacity < self->size + 1)
	{
		ent_resize(self, self->capacity + (self->size + 1) / 2);
	}

	struct enode ent;
	ent.name = name;
	ent.owns_name = owns_name;
	lstat(ent.name, &sb);
	ent.mode = sb.st_mode;

	self->data[self->size] = ent;
	self->size++;
}

void ent_args(struct ent_vec *self, int count, char *values[])
{
	int ind;

	ent_resize(self, (size_t)count);
	for (ind = 0; ind < count; ind++)
	{
		ent_add(self, values[ind], false);
	}
}

void ent_clear(struct ent_vec *self)
{
	self->size = 0;
}

void enode_print(struct enode *ent)
{
	puts(ent->name);
}

void enode_print0(struct enode *ent)
{
	printf("%s", ent->name);
	putchar('\0');
}

void ent_glob(struct ent_vec *self, glob_t *globbuf)
{
	struct enode *ent = ent_beg(self);

	glob(ent->name, GLOB_TILDE, NULL, globbuf);
	while (++ent != ent_end(self))
	{
		glob(ent->name, GLOB_TILDE | GLOB_APPEND, NULL, globbuf);
	} 
	ent_clear(self);
	ent_args(self, globbuf->gl_pathc, globbuf->gl_pathv);
}

/* Creates a path form two parts. Allocated memory must be freed manually. */
char *join_path(char *part1, char *part2)
{
	char *path = malloc(strlen(part1) + strlen(part2) + 3);
	strcpy(path, part1);
	strcat(path, "/");
	strcat(path, part2);

	return path;
}

void readdir_ent(struct ent_vec *self, struct enode *ent)
{
	if (!S_ISDIR(ent->mode)) return;
	char *path = ent->name;

	struct dirent *dirent;
	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		perror("opendir");
		//abort();
		return;
	}

	while ((dirent = readdir(dir)) != NULL)
	{
		if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
		{
			continue;
		}

		ent_add(self, join_path(path, dirent->d_name), true);
	}
	closedir(dir);
}

void ent_recurse(struct ent_vec *self)
{
	size_t ind;
	
	for (ind = 0; ind < self->size; ind++)
	{
		readdir_ent(self, ent_at(self, ind));
	}
}

int main(int argc, char *argv[])
{
	int opt;
	size_t ind;
	glob_t globbuf;
	struct ent_vec ents;

	while ((opt = getopt(argc, argv, "drz0")) != -1)
	{
		switch (opt)
		{
			case 'd':
				options |= OPT_DIR;
				break;
			case 'r':
				options |= OPT_RECURSIVE;
				break;
			case 'z':
			case '0':
				options |= OPT_NULL;
			break;
			default:
				abort();
				
		}
	}
	/* process other arguments */
	argc -= optind;
	argv += optind;

	ent_init(&ents, argc);
	ent_args(&ents, argc, argv);
	ent_glob(&ents, &globbuf);
	if (options & OPT_RECURSIVE)
	{	
		ent_recurse(&ents);
	}
	for (ind = 0; ind < ents.size; ind++)
	{
		struct enode *ent = ent_at(&ents, ind);
		if (!S_ISDIR(ent->mode) || (options & OPT_DIR))
		{
			if (options & OPT_NULL)
			{
				printf("%s", ent->name);
				putchar('\0');
			}
			else
			{
				puts(ent->name);
			}
		}
	}
	ent_free(&ents);
	globfree(&globbuf);
}
