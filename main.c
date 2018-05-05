#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"


void
strip(char *s)
{
	char *b, *e;

	b = s;

	while (*b == ' ' || *b == '\t')
		if (*b++ == '\0') {
			s[0] = '\0';
			return;
		}

	e = s + strlen(s) - 1;

	while (*e-- == ' ');

	e++;

//	printf("strlen %d, b = %d, mod = %d\n", strlen(s), b - s, e - b);
	*++e = '\0';
	memmove(s, b, e - b + 1);
}

char
dgetchar()
{
	int c;

	c = getchar();

	if (c == EOF)
		exit(0);

	return c;
}

size_t
hash(void * const a, size_t len)
{
	(void) a, len;

	return 0;
}

int
main(void)
{
	char *c;
	char k[100];
	char v[100];
	dict a;

	dict_init(&a, hash, realloc);

	for (;;) {
	beg:
		printf(">> ");

		c = k;

		while ((*c = dgetchar()) != '=')
			if (*c++ == '\n') {
				*--c = '\0';
				strip(k);
				if (strcmp(k, "debug") == 0)
					dict_print(&a);
				else
					printf("'%s' is '%s'\n", k, (char *)(dict_get(&a, k, strlen(k)).p));
				goto beg;
			}

		*c = '\0';

		c = v;
		while ((*c++ = dgetchar()) != '\n');
		*--c = '\0';

		strip(k);
		strip(v);
		printf("SETTING '%s' to '%s'\n", k, v);
		dict_set(&a, k, strlen(k) + 1, v, strlen(v) + 1);
	}

	printf("len = %lu\n", a.len);
	dict_free(&a);

	return 0;
}

