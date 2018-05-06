#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "dict.h"
#include "vector/vector.h"
#include "primes.h"


size_t
djb2(void * const p, size_t len)
{
	unsigned long hash = 5381;
	int c;
	char *str = p;

	while (len--) {
		c = *str++;
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

int
dict_init(dict *d, size_t (*hash)(void * const, size_t),
		void *(*alloc)(void *, size_t))
{
	size_t i;

	assert(d);

	d->len = 0;
	d->mod = prime_nearest(0);
	d->hash = hash == NULL ? djb2 : hash;
	d->alloc = alloc;
	d->vec = alloc ? d->alloc(NULL, sizeof(vector) * d->mod) : malloc(sizeof(vector) * d->mod);
	d->iter_cnt = 0;

	if (d->vec == NULL)
		return DICT_MEM_ERR;

	for (i = 0; i < d->mod; i++) {
		if (vector_init(d->vec + i, sizeof(dict_ent), alloc) == VECTOR_MEM_ERR) {
			while (i--)
				vector_free(d->vec + i);

			alloc ? d->alloc(d->vec, 0) : free(d->vec);

			return DICT_MEM_ERR;
		}
	}

	return DICT_OK;
}

dict_ent
dict_get_ent(const dict *d, void * const key, size_t key_len)
{
	size_t n, i;
	dict_ent *vec;
	dict_ent null;

	assert(d);
	assert(d->vec);
	assert(d->mod);
	assert(d->hash);
	assert(key);
	assert(key_len);

	n = d->hash(key, key_len) % d->mod;
	vec = vector_data(d->vec + n);

	assert(vec);

	for (i = 0; i < vector_nmemb(d->vec + n); i++) {
		if (vec[i].key.len == key_len &&
				strncmp(vec[i].key.p, key, key_len) == 0)
			return vec[i];
	}

	null.key.p = null.val.p = NULL;
	null.key.len = null.val.len = 0;

	return null;
}

dict_obj
dict_get(const dict *d, void * const key, size_t key_len)
{
	return dict_get_ent(d, key, key_len).val;
}

void
dict_remove(dict *d, void * const key, size_t key_len)
{
	size_t n, i;
	dict_ent *vec;

	assert(d);
	assert(d->vec);
	assert(d->mod);
	assert(d->hash);
	assert(d->iter_cnt == 0);
	assert(key);
	assert(key_len);

	n = d->hash(key, key_len) % d->mod;
	vec = vector_data(d->vec + n);

	assert(vec);

	for (i = 0; i < vector_nmemb(d->vec + n); i++) {
		if (vec[i].key.len == key_len &&
				strncmp(vec[i].key.p, key, key_len) == 0) {

			vector_remove(d->vec + n, i);
			return;
		}
	}
}

void
dict_free(dict *d)
{
	size_t i, j;
	dict_ent *vec;

	assert(d);
	assert(d->vec);
	assert(d->mod);
	assert(d->iter_cnt == 0);

	if (!d->alloc)
		goto end;

	for (i = 0; i < d->mod; i++) {
		vec = vector_data(d->vec + i);

		for (j = 0; j < vector_nmemb(d->vec); j++) {
			d->alloc(vec[j].val.p, 0);
			d->alloc(vec[j].key.p, 0);
		}

		vector_free(d->vec + i);
	}

end:
	if (d->alloc)
		d->alloc(d->vec, 0);
	else
		free(d->vec);
}

dict_ent
dict_iterate(struct dict_iter *iter)
{
	dict_ent ent;
	dict_ent *p;

	assert(iter);
	assert(iter->d);
	assert(iter->d->mod);
	assert(iter->d->vec);
	assert(iter->d->iter_cnt > 0);

	ent.val.p = ent.key.p = NULL;
	ent.val.len = ent.key.len = 0;

	while (iter->i < iter->d->mod &&
	    iter->j >= vector_nmemb(iter->d->vec + iter->i)) {

		iter->i++;
		iter->j = 0;
	}

	if (iter->i >= iter->d->mod) {
		iter->d->iter_cnt--;
		iter->d = NULL;
		return ent;
	}

	p = (dict_ent *)vector_get(iter->d->vec + iter->i, iter->j);

	assert(p);

	ent = *p;
	iter->j++;

	return ent;
}

void
dict_iter_init(dict * const d, struct dict_iter *iter)
{
	assert(d);
	assert(d->mod);

	d->iter_cnt++;

	iter->d = d;
	iter->i = iter->j = 0;
}

int
dict_resize(dict *a)
{
	dict b;
	dict_ent o;
	struct dict_iter iter;
	size_t i;

	assert(a);
	assert(a->vec);
	assert(a->mod);
	assert(a->hash);
	assert(a->iter_cnt == 0);

	b.len = 0;
	b.mod = prime_nearest(a->mod + 1);
	b.hash = a->hash;
	b.alloc = NULL;
	b.vec = a->alloc(NULL, sizeof(vector) * b.mod);

	if (b.vec == NULL)
		return DICT_MEM_ERR;

	for (i = 0; i < b.mod; i++) {
		if (vector_init(b.vec + i, sizeof(dict_ent), a->alloc) != VECTOR_OK) {
			while (i--)
				vector_free(b.vec + i);

			vector_free(b.vec);

			return DICT_MEM_ERR;
		}
	}

	dict_iter_init(a, &iter);

	for (o = dict_iterate(&iter); o.key.p != NULL; o = dict_iterate(&iter)) {
		if (dict_set(&b, o.key.p, o.key.len, o.val.p, o.val.len) != DICT_OK) {
			dict_free(&b);
			return DICT_MEM_ERR;
		}
	}

	b.alloc = a->alloc;
	a->alloc = NULL;

	dict_free(a);

	memcpy(a, &b, sizeof(b));

	return DICT_OK;
}

int
dict_set(dict *d, void * const key, size_t key_len, void *val, size_t val_len)
{
	size_t n, i;
	dict_ent *vec, obj;

	assert(d);
	assert(d->vec);
	assert(d->mod);
	assert(d->hash);
	assert(d->iter_cnt == 0);
	assert(key);
	assert(key_len);

	if (val == NULL) {
		dict_remove(d, key, key_len);

		return DICT_OK;
	}

	n = d->hash(key, key_len) % d->mod;
	vec = vector_data(d->vec + n);

	assert(vec);

	for (i = 0; i < vector_nmemb(d->vec + n); i++) {
		if (vec[i].key.len != key_len)
			continue;
		if (strncmp(vec[i].key.p, key, key_len) != 0)
			continue;

		if (d->alloc) {
			obj.val.p = d->alloc(NULL, val_len);
			obj.val.len = val_len;

			if (obj.val.p == NULL)
				return DICT_MEM_ERR;

			d->alloc(vec[i].val.p, 0);
			vec[i].val = obj.val;

			memcpy(vec[i].val.p, val, val_len);
		} else {
			vec[i].val.p = val;
			vec[i].val.len = val_len;
		}

		return DICT_OK;
	}

	/* Adding new element */
	if (d->alloc == NULL) {
		obj.key.p = key;
		obj.key.len = key_len;
		obj.val.p = val;
		obj.val.len = val_len;

		if (vector_push(d->vec + n, &obj) != VECTOR_OK)
			return DICT_MEM_ERR;

		d->maxdepth = d->maxdepth > vector_nmemb(d->vec + n) ?
			d->maxdepth : vector_nmemb(d->vec + n);
		d->len++;

		return DICT_OK;
	}

	obj.key.p = d->alloc(NULL, key_len);
	obj.key.len = key_len;

	if (obj.key.p == NULL)
		return DICT_MEM_ERR;

	obj.val.p = d->alloc(NULL, val_len);
	obj.val.len = val_len;

	if (obj.val.p == NULL) {
		d->alloc(obj.key.p, 0);
		return DICT_MEM_ERR;
	}

	if (vector_push(d->vec + n, &obj) != VECTOR_OK) {
		d->alloc(obj.key.p, 0);
		d->alloc(obj.val.p, 0);
		return DICT_MEM_ERR;
	}

	memcpy(obj.key.p, key, key_len);
	memcpy(obj.val.p, val, val_len);

	d->len++;

	d->maxdepth = d->maxdepth > vector_nmemb(d->vec + n) ?
		d->maxdepth : vector_nmemb(d->vec + n);

	if ((float)d->len / (float)d->mod > 2.0) {
		dict_resize(d);
		return DICT_OK;
	}

	return DICT_OK;
}

size_t
dict_len(dict *d)
{
	assert(d);

	return d->len;
}

void
dict_print(dict *d)
{
	size_t i, j;
	dict_ent *vec;

	assert(d);
	assert(d->vec);
	assert(d->mod);

	printf("\nFANCY PRINT:\n\n");

	for (i = 0; i < d->mod; i++) {
		if (vector_nmemb(d->vec + i) == 0)
			continue;

		vec = vector_data(d->vec + i);

		printf("arr[%lu] = {\n", i);

		for (j = 0; j < vector_nmemb(d->vec + i); j++) {
			printf("\t'%s' : '%s'\n",
					(char *)vec[j].key.p,
					(char *)vec[j].val.p);
		}
		printf("}\n\n");
	}

	printf("TOTAL: %lu\n", d->len);
	printf("MODULUS: %lu\n", d->mod);
}

