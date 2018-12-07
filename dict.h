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
	int iter_cnt;

} dict;

typedef struct {
	const dict *d;
	size_t i, j;

} dict_iter;


dict *dict_init(int copy_key);
void dict_free(dict *d);

void *dict_get(const dict *d, const char *key);
int dict_set(dict *d, const char *key, const void *val);
void dict_remove(dict *d, const char *key);

size_t dict_len(dict *d);

dict_iter *dict_iter_init(dict *d);
struct dict_ent *dict_iterate(dict_iter *it);
void dict_iter_free(dict_iter *it);


#endif /* _DICT_H_ */

