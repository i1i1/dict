#ifndef _DICT_H_
#define _DICT_H_

#include <stdlib.h>

#define DICT_REALLOC(ptr, size)	(realloc(ptr, size))
#define DICT_FREE(ptr)			(free(ptr))
#define DICT_MALLOC(size)		(malloc(size))


struct dict_ent {
	const char *key;
	size_t hkey;

	const void *val;
};

typedef struct {
	struct dict_ent **vec;
	size_t len; // Number of entries in dict
	size_t mod; // Number of entries in vec
//	size_t maxdepth; // Maximum length of vectors in dict.vec
	int copy_key;

} dict;


dict *dict_init(int copy_key);

void dict_free(dict *d);

void *dict_get(const dict *d, const char *key);

int dict_set(dict *d, const char *key, const void *val);

void dict_remove(dict *d, const char *key);

size_t dict_len(dict *d);


#endif /* _DICT_H_ */

