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

void ent_add(struct ent_vec *self, struct enode ent)
{
	struct stat sb;

	if (self->capacity < self->size + 1)
	{
		ent_resize(self, self->capacity + (self->size + 1) / 2);
	}

	stat(ent.name, &sb);
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
		struct enode ent;
		ent.name = values[ind];
		ent.owns_name = false;

		ent_add(self, ent);
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

void readdir_ent(struct ent_vec *self, struct enode *ent)
{
	if (!S_ISDIR(ent->mode)) return;

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

		struct enode ent2;
		ent2.name = name;
		ent2.owns_name = true;
		ent_add(self, ent2);

		if (dirent->d_type == DT_DIR)
		{
			readdir_ent(self, ent_at(self, self->size - 1));
		}

		entc++;
	}
	closedir(dir);
}

void ent_recurse(struct ent_vec *self)
{
	readdir_ent(self, ent_beg(self));
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

	ent_init(&ents, argc - optind);
	ent_args(&ents, argc - optind, argv + optind);
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
