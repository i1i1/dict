#ifndef _DICT_H_
#define _DICT_H_

#include <stdlib.h>

#include "vector/vector.h"

enum {
	DICT_OK = 0,
	DICT_MEM_ERR = 1,
};

typedef struct {
	void *p;
	size_t len;
} dict_obj;

typedef struct {
	dict_obj key;
	dict_obj val;
} dict_ent;

typedef struct {
	vector *vec;
	size_t len; // Number of entries in dict
	size_t mod; // Number of entries in vec
	size_t maxdepth; // Maximum length of vectors in dict.vec
	size_t iter_cnt; // Number of unfinished iterators in dict
	size_t (*hash)(void * const, size_t);
	void *(*alloc)(void *, size_t);
} dict;

struct dict_iter {
	dict *d;
	size_t i;
	size_t j;
};


int dict_init(dict *d, size_t (*hash)(void * const, size_t),
	      void *(*alloc)(void *, size_t));

void dict_free(dict *d);

dict_ent dict_get_ent(const dict *d, void * const key, size_t key_len);

dict_obj dict_get(const dict *d, void * const key, size_t key_len);

int dict_set(dict *d, void * const key, size_t key_len, void *val, size_t val_len);

void dict_remove(dict *d, void * const key, size_t key_len);

dict_ent dict_iterate(struct dict_iter *iter);

void dict_iter_init(dict * const d, struct dict_iter *iter);

int dict_resize(dict *a);

size_t dict_len(dict *d);

void dict_print(dict *d);

#endif /* _DICT_H_ */
