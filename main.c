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

int
main(void)
{
	char *c;
	char k[100];
	char v[100];
	dict *a;

	a = dict_init(1);

	if (!a)
		exit(1);

	for (;;) {
	beg:
		printf(">> ");

		c = k;

		while ((*c = dgetchar()) != '=')
			if (*c++ == '\n') {
				*--c = '\0';
				strip(k);
				printf("'%s' is '%s'\n", k, (char *)(dict_get(a, k)));
				goto beg;
			}

		*c = '\0';

		c = v;
		while ((*c++ = dgetchar()) != '\n');
		*--c = '\0';

		strip(k);
		strip(v);
		printf("SETTING '%s' to '%s'\n", k, v);
		if (dict_set(a, k, v))
			exit(1);
	}

	printf("len = %lu\n", dict_len(a));
	dict_free(a);

	return 0;
}

