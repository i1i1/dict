#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dict.h"
#include "vector/vector.h"

#define BUFSIZE	256


void
error(char *msg)
{
	puts(msg);
	exit(1);
}

int
dict_add(dict *d, char *w)
{
	dict_get(d, w);

	if (dict_get(d, w))
		return dict_set(d, w, (void *)((size_t)dict_get(d, w) + 1));
	else
		return dict_set(d, w, (void *)0x1);
}

int
freq_cmp(const void *aa, const void *bb)
{
    const struct dict_ent *a = aa;
    const struct dict_ent *b = bb;

	return (size_t)a->val - (size_t)b->val;
}

struct dict_ent *
get_freqs(dict *d)
{
	dict_iter *it;
	struct dict_ent *e, *res;

	if (!(it = dict_iter_init(d)))
		error("No memory");

	res = NULL;

	while ((e = dict_iterate(it))) {
		if (vector_push(res, *e))
			error("No memory");
	}

	dict_iter_free(it);

	return res;
}

int
main(int argc, char **argv)
{
	dict *d;
	FILE *fp;
	size_t i;
	struct dict_ent *farr;

	if (!(d = dict_init(1)))
		error("No memory");

	if (argc <= 1)
		error("No arguments");

	for (i = 1; i < (unsigned)argc; i++) {
		char a[BUFSIZE];
		int c, j;

		if (!(fp = fopen(argv[i], "r")))
			error("Can't open file");

		j = 0;
		while ((c = getc(fp)) != EOF) {
			if (!isalnum(c) && c != '-') {
				if (j > 0) {
					a[j] = '\0';
					j = 0;
					if (dict_add(d, a))
						error("No memmory!");
				}
				continue;
			}
			a[j++] = tolower(c);
		}
		if (fp != stdin)
			fclose(fp);
	}

	farr = get_freqs(d);
	qsort(farr, vector_nmemb(farr), sizeof(*farr), freq_cmp);

	for (i = 0; i < vector_nmemb(farr); i++)
			printf("'%s' -> %ld\n", farr[i].key, (size_t)farr[i].val);

	dict_free(d);

	return 0;
}

