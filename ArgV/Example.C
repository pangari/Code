#define VERBOSE=1

#include "ArgV.H"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

const struct argv_test {
    const char *cmdline;
    const char *argv[10];
} argv_tests[] = {
    /*
     * We generate this set of tests by invoking ourself with
     * `-generate'.
     */
    {"ab c\" d", {"ab", "c d", nullptr}},
    {"a\"b c\" d", {"ab c", "d", nullptr}},
    {"a\"\"b c\" d", {"ab", "c d", nullptr}},
    {"a\"\"\"b c\" d", {"a\"b", "c d", nullptr}},
    {"a\"\"\"\"b c\" d", {"a\"b c", "d", nullptr}},
    {"a\"\"\"\"\"b c\" d", {"a\"b", "c d", nullptr}},
    {"a\"\"\"\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"a\"\"\"\"\"\"\"b c\" d", {"a\"\"b c", "d", nullptr}},
    {"a\"\"\"\"\"\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"a\\b c\" d", {"a\\b", "c d", nullptr}},
    {"a\\\"b c\" d", {"a\"b", "c d", nullptr}},
    {"a\\\"\"b c\" d", {"a\"b c", "d", nullptr}},
    {"a\\\"\"\"b c\" d", {"a\"b", "c d", nullptr}},
    {"a\\\"\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"a\\\"\"\"\"\"b c\" d", {"a\"\"b c", "d", nullptr}},
    {"a\\\"\"\"\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"a\\\"\"\"\"\"\"\"b c\" d", {"a\"\"\"b", "c d", nullptr}},
    {"a\\\"\"\"\"\"\"\"\"b c\" d", {"a\"\"\"b c", "d", nullptr}},
    {"a\\\\b c\" d", {"a\\\\b", "c d", nullptr}},
    {"a\\\\\"b c\" d", {"a\\b c", "d", nullptr}},
    {"a\\\\\"\"b c\" d", {"a\\b", "c d", nullptr}},
    {"a\\\\\"\"\"b c\" d", {"a\\\"b", "c d", nullptr}},
    {"a\\\\\"\"\"\"b c\" d", {"a\\\"b c", "d", nullptr}},
    {"a\\\\\"\"\"\"\"b c\" d", {"a\\\"b", "c d", nullptr}},
    {"a\\\\\"\"\"\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"a\\\\\"\"\"\"\"\"\"b c\" d", {"a\\\"\"b c", "d", nullptr}},
    {"a\\\\\"\"\"\"\"\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"a\\\\\\b c\" d", {"a\\\\\\b", "c d", nullptr}},
    {"a\\\\\\\"b c\" d", {"a\\\"b", "c d", nullptr}},
    {"a\\\\\\\"\"b c\" d", {"a\\\"b c", "d", nullptr}},
    {"a\\\\\\\"\"\"b c\" d", {"a\\\"b", "c d", nullptr}},
    {"a\\\\\\\"\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"a\\\\\\\"\"\"\"\"b c\" d", {"a\\\"\"b c", "d", nullptr}},
    {"a\\\\\\\"\"\"\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"a\\\\\\\"\"\"\"\"\"\"b c\" d", {"a\\\"\"\"b", "c d", nullptr}},
    {"a\\\\\\\"\"\"\"\"\"\"\"b c\" d", {"a\\\"\"\"b c", "d", nullptr}},
    {"a\\\\\\\\b c\" d", {"a\\\\\\\\b", "c d", nullptr}},
    {"a\\\\\\\\\"b c\" d", {"a\\\\b c", "d", nullptr}},
    {"a\\\\\\\\\"\"b c\" d", {"a\\\\b", "c d", nullptr}},
    {"a\\\\\\\\\"\"\"b c\" d", {"a\\\\\"b", "c d", nullptr}},
    {"a\\\\\\\\\"\"\"\"b c\" d", {"a\\\\\"b c", "d", nullptr}},
    {"a\\\\\\\\\"\"\"\"\"b c\" d", {"a\\\\\"b", "c d", nullptr}},
    {"a\\\\\\\\\"\"\"\"\"\"b c\" d", {"a\\\\\"\"b", "c d", nullptr}},
    {"a\\\\\\\\\"\"\"\"\"\"\"b c\" d", {"a\\\\\"\"b c", "d", nullptr}},
    {"a\\\\\\\\\"\"\"\"\"\"\"\"b c\" d", {"a\\\\\"\"b", "c d", nullptr}},
    {"\"ab c\" d", {"ab c", "d", nullptr}},
    {"\"a\"b c\" d", {"ab", "c d", nullptr}},
    {"\"a\"\"b c\" d", {"a\"b", "c d", nullptr}},
    {"\"a\"\"\"b c\" d", {"a\"b c", "d", nullptr}},
    {"\"a\"\"\"\"b c\" d", {"a\"b", "c d", nullptr}},
    {"\"a\"\"\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"\"a\"\"\"\"\"\"b c\" d", {"a\"\"b c", "d", nullptr}},
    {"\"a\"\"\"\"\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"\"a\"\"\"\"\"\"\"\"b c\" d", {"a\"\"\"b", "c d", nullptr}},
    {"\"a\\b c\" d", {"a\\b c", "d", nullptr}},
    {"\"a\\\"b c\" d", {"a\"b c", "d", nullptr}},
    {"\"a\\\"\"b c\" d", {"a\"b", "c d", nullptr}},
    {"\"a\\\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"\"a\\\"\"\"\"b c\" d", {"a\"\"b c", "d", nullptr}},
    {"\"a\\\"\"\"\"\"b c\" d", {"a\"\"b", "c d", nullptr}},
    {"\"a\\\"\"\"\"\"\"b c\" d", {"a\"\"\"b", "c d", nullptr}},
    {"\"a\\\"\"\"\"\"\"\"b c\" d", {"a\"\"\"b c", "d", nullptr}},
    {"\"a\\\"\"\"\"\"\"\"\"b c\" d", {"a\"\"\"b", "c d", nullptr}},
    {"\"a\\\\b c\" d", {"a\\\\b c", "d", nullptr}},
    {"\"a\\\\\"b c\" d", {"a\\b", "c d", nullptr}},
    {"\"a\\\\\"\"b c\" d", {"a\\\"b", "c d", nullptr}},
    {"\"a\\\\\"\"\"b c\" d", {"a\\\"b c", "d", nullptr}},
    {"\"a\\\\\"\"\"\"b c\" d", {"a\\\"b", "c d", nullptr}},
    {"\"a\\\\\"\"\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"\"a\\\\\"\"\"\"\"\"b c\" d", {"a\\\"\"b c", "d", nullptr}},
    {"\"a\\\\\"\"\"\"\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"\"a\\\\\"\"\"\"\"\"\"\"b c\" d", {"a\\\"\"\"b", "c d", nullptr}},
    {"\"a\\\\\\b c\" d", {"a\\\\\\b c", "d", nullptr}},
    {"\"a\\\\\\\"b c\" d", {"a\\\"b c", "d", nullptr}},
    {"\"a\\\\\\\"\"b c\" d", {"a\\\"b", "c d", nullptr}},
    {"\"a\\\\\\\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"\"a\\\\\\\"\"\"\"b c\" d", {"a\\\"\"b c", "d", nullptr}},
    {"\"a\\\\\\\"\"\"\"\"b c\" d", {"a\\\"\"b", "c d", nullptr}},
    {"\"a\\\\\\\"\"\"\"\"\"b c\" d", {"a\\\"\"\"b", "c d", nullptr}},
    {"\"a\\\\\\\"\"\"\"\"\"\"b c\" d", {"a\\\"\"\"b c", "d", nullptr}},
    {"\"a\\\\\\\"\"\"\"\"\"\"\"b c\" d", {"a\\\"\"\"b", "c d", nullptr}},
    {"\"a\\\\\\\\b c\" d", {"a\\\\\\\\b c", "d", nullptr}},
    {"\"a\\\\\\\\\"b c\" d", {"a\\\\b", "c d", nullptr}},
    {"\"a\\\\\\\\\"\"b c\" d", {"a\\\\\"b", "c d", nullptr}},
    {"\"a\\\\\\\\\"\"\"b c\" d", {"a\\\\\"b c", "d", nullptr}},
    {"\"a\\\\\\\\\"\"\"\"b c\" d", {"a\\\\\"b", "c d", nullptr}},
    {"\"a\\\\\\\\\"\"\"\"\"b c\" d", {"a\\\\\"\"b", "c d", nullptr}},
    {"\"a\\\\\\\\\"\"\"\"\"\"b c\" d", {"a\\\\\"\"b c", "d", nullptr}},
    {"\"a\\\\\\\\\"\"\"\"\"\"\"b c\" d", {"a\\\\\"\"b", "c d", nullptr}},
    {"\"a\\\\\\\\\"\"\"\"\"\"\"\"b c\" d", {"a\\\\\"\"\"b", "c d", nullptr}},
};

#define lenof(arr) sizeof(arr) / sizeof(arr[0])

int main(int argc, char **argv)
{
    int i, j;

    if (argc > 1) {
	/*
	 * Generation of tests.
	 * 
	 * Given `-splat <args>', we print out a C-style
	 * representation of each argument (in the form "a", "b",
	 * nullptr), backslash-escaping each backslash and double
	 * quote.
	 * 
	 * Given `-split <string>', we first doctor `string' by
	 * turning forward slashes into backslashes, single quotes
	 * into double quotes and underscores into spaces; and then
	 * we feed the resulting string to ourself with `-splat'.
	 * 
	 * Given `-generate', we concoct a variety of fun test
	 * cases, encode them in quote-safe form (mapping \, " and
	 * space to /, ' and _ respectively) and feed each one to
	 * `-split'.
	 */
	if (!strcmp(argv[1], "-splat")) {
	    int i;
	    char *p;
	    for (i = 2; i < argc; i++) {
		putchar('"');
		for (p = argv[i]; *p; p++) {
		    if (*p == '\\' || *p == '"')
			putchar('\\');
		    putchar(*p);
		}
		printf("\", ");
	    }
	    printf("nullptr");
	    return 0;
	}

	if (!strcmp(argv[1], "-split") && argc > 2) {
	    char *str = (char*)malloc(20 + strlen(argv[0]) + strlen(argv[2]));
	    char *p, *q;

	    q = str + sprintf(str, "%s -splat ", argv[0]);
	    printf("    {\"");
	    for (p = argv[2]; *p; p++, q++) {
		switch (*p) {
		  case '/':  printf("\\\\"); *q = '\\'; break;
		  case '\'': printf("\\\""); *q = '"';  break;
		  case '_':  printf(" ");    *q = ' ';  break;
		  default:   putchar(*p);    *q = *p;   break;
		}
	    }
	    *p = '\0';
	    printf("\", {");
	    fflush(stdout);

	    system(str);

	    printf("}},\n");

	    return 0;
	}

	if (!strcmp(argv[1], "-generate")) {
	    char *teststr, *p;
	    int i, initialquote, backslashes, quotes;

	    teststr = (char*)malloc(200 + strlen(argv[0]));

	    for (initialquote = 0; initialquote <= 1; initialquote++) {
		for (backslashes = 0; backslashes < 5; backslashes++) {
		    for (quotes = 0; quotes < 9; quotes++) {
			p = teststr + sprintf(teststr, "%s -split ", argv[0]);
			if (initialquote) *p++ = '\'';
			*p++ = 'a';
			for (i = 0; i < backslashes; i++) *p++ = '/';
			for (i = 0; i < quotes; i++) *p++ = '\'';
			*p++ = 'b';
			*p++ = '_';
			*p++ = 'c';
			*p++ = '\'';
			*p++ = '_';
			*p++ = 'd';
			*p = '\0';

			printf("[%s]\n", teststr);
		    }
		}
	    }
	    return 0;
	}

	fprintf(stderr, "unrecognised option: \"%s\"\n", argv[1]);
	return 1;
    }

    /*
     * If we get here, we were invoked with no arguments, so just
     * run the tests.
     */

    for (i = 0; i < lenof(argv_tests); i++) {
	int ac;
	char **av;

	SplitIntoArgV(argv_tests[i].cmdline, ac, &av);

	for (j = 0; j < ac && argv_tests[i].argv[j]; j++) {
	    if (strcmp(av[j], argv_tests[i].argv[j])) {
		printf("failed test %d (|%s|) arg %d: |%s| should be |%s|\n",
		       i, argv_tests[i].cmdline,
		       j, av[j], argv_tests[i].argv[j]);
	    }
#ifdef VERBOSE
	    else {
		printf("test %d (|%s|) arg %d: |%s| == |%s|\n",
		       i, argv_tests[i].cmdline,
		       j, av[j], argv_tests[i].argv[j]);
	    }
#endif
	}
	if (j < ac)
	    printf("failed test %d (|%s|): %d args returned, should be %d\n",
		   i, argv_tests[i].cmdline, ac, j);
	if (argv_tests[i].argv[j])
	    printf("failed test %d (|%s|): %d args returned, should be more\n",
		   i, argv_tests[i].cmdline, ac);
    }

    return 0;
}
