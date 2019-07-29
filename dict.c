#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define VECTOR_IMPLEMENTATION
#include "vector/vector.h"

#include "dict.h"
#include "primes.h"


#define STREQ(a, b)		(strcmp(a, b) == 0)


static size_t
hash(const char *s)
{
	unsigned long h = 5381;
	int c;

	while ((c = *s++))
		h = ((h << 5) + h) + c; /* h * 33 + c */

	return h;
}

dict *
dict_init(int copy_key)
{
	dict *d;

	d = DICT_MALLOC(sizeof(*d));

	if (!d)
		return NULL;

	d->len = 0;
	d->mod = prime_nearest(0);
	d->copy_key = copy_key;
	d->iter_cnt = 0;
	d->vec = DICT_MALLOC(sizeof(*d->vec) * d->mod);

	if (!d->vec) {
		DICT_FREE(d);
		return NULL;
	}

	memset(d->vec, 0, sizeof(*d->vec) * d->mod);

	return d;
}

void *
dict_get(const dict *d, const char *key)
{
	size_t i;
	size_t h;
	struct dict_ent *vec;

	assert(d);
	assert(d->vec);
	assert(key);

	h = hash(key);
	vec = d->vec[h % d->mod];

	if (vec == NULL)
		return NULL;

	for (i = 0; i < vector_nmemb(&vec); i++) {
		if (h == vec[i].hkey && STREQ(vec[i].key, key))
			return (void *)vec[i].val;
	}

	return NULL;
}

static void
vector_remove(struct dict_ent *v, size_t n)
{
	assert(v);
	assert(n < vector_nmemb(&v));

	v[n] = vector_pop(&v);
}

void
dict_remove(dict *d, const char *key)
{
	size_t h, i;
	struct dict_ent *vec;

	assert(d);
	assert(d->vec);
	assert(key);

	h = hash(key);
	vec = d->vec[h % d->mod];

	for (i = 0; i < vector_nmemb(&vec); i++) {
		if (vec[i].hkey == h && STREQ(vec[i].key, key)) {
			vector_remove(vec, i);
			d->len--;
			return;
		}
	}
}

void
dict_free(dict *d)
{
	size_t i, j;
	struct dict_ent *vec;

	assert(d);
	assert(d->vec);

	for (i = 0; i < d->mod; i++) {
		vec = d->vec[i];

		for (j = 0; j < vector_nmemb(&vec); j++) {
			if (d->copy_key)
				DICT_FREE((void *)vec[j].key);
		}

		vector_free(d->vec + i);
	}

	DICT_FREE(d->vec);
	DICT_FREE(d);
}

static int
dict_forceadd(dict *d, struct dict_ent ent)
{
	if (vector_push(&(d->vec[ent.hkey % d->mod]), ent))
		return 1;

	d->len++;

	return 0;
}

int
dict_resize(dict *a)
{
	dict b;
	struct dict_ent *vec;
	size_t i, j;

	assert(a);
	assert(a->vec);

	b.len = 0;
	b.mod = prime_nearest(a->mod + 1);
	b.vec = DICT_MALLOC(sizeof(*b.vec) * b.mod);
	b.copy_key = a->copy_key;


	if (!b.vec)
		return 1;

	memset(b.vec, 0, sizeof(*b.vec) * b.mod);

	for (i = 0; i < a->mod; i++) {
		vec = a->vec[i];
		for (j = 0; j < vector_nmemb(&vec); j++) {
			if (dict_forceadd(&b, vec[j]))
				return 1;
		}
		vector_free(a->vec + i);
	}

	DICT_FREE(a->vec);
	*a = b;

	return 0;
}

int
dict_set(dict *d, const char *key, const void *val)
{
	size_t h, i;
	struct dict_ent *vec;
	struct dict_ent ent;
	char *s;

	assert(d);
	assert(d->vec);
	assert(key);

	h = hash(key);
	vec = d->vec[h % d->mod];

	for (i = 0; i < vector_nmemb(&vec); i++) {
		if (h != vec[i].hkey || !STREQ(vec[i].key, key))
			continue;

		vec[i].val = val;

		return 0;
	}

	if (d->len / d->mod > 4) {
		if (dict_resize(d))
			return 1;
	}

	/* Adding new element */

	if (d->copy_key) {
		int l;

		l = strlen(key) + 1;

		if (!(s = DICT_MALLOC(l)))
			return 1;

		memcpy(s, key, l);

	} else
		s = (char *)key;

	ent.key = s;
	ent.hkey = h;
	ent.val = val;

	return dict_forceadd(d, ent);
}

size_t
dict_len(dict *d)
{
	assert(d);

	return d->len;
}

dict_iter *
dict_iter_init(dict *d)
{
	dict_iter *it;

	assert(d);

	it = DICT_MALLOC(sizeof(*it));

	if (!it)
		return NULL;

	it->d = d;
	it->i = 0;
	it->j = 0;
	d->iter_cnt++;

	return it;
}

struct dict_ent *
dict_iterate(dict_iter *it)
{
	struct dict_ent *vec;
	size_t i;

	assert(it);
	assert(it->d);
	assert(it->d->vec);

	if (it->i >= it->d->mod)
		return NULL;

	vec = it->d->vec[it->i];

	if (it->j < vector_nmemb(&vec))
		return vec + it->j++;

	for (i = it->i + 1; i < it->d->mod; i++) {
		if (vector_nmemb(it->d->vec + i)) {
			it->i = i;
			it->j = 1;
			return it->d->vec[i];
		}
	}

	return NULL;
}

void
dict_iter_free(dict_iter *it)
{
	dict *d;

	assert(it);
	assert(it->d);

	d = (dict *)it->d;
	d->iter_cnt--;

	DICT_FREE(it);
}

