///////////////////////////////////////////////////////////////////////////////
//
// Advent of Code 2019
// Day 5b: Intcode program expanded more
//
// E. Dronkert
// https://github.com/ednl/aoc2019
//
///////////////////////////////////////////////////////////////////////////////

////////// Includes & Defines /////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

////////// Typedefs & Constants ///////////////////////////////////////////////

typedef struct Pair {
	int around, inorbit;
} PAIR, PPAIR;

const char *inp = "inp06.txt";

////////// Functions //////////////////////////////////////////////////////////

// Count lines in a multi-line text file
int size(void)
{
	FILE *fp;
	char *s = NULL;
	size_t t = 0;
	int count = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		while (getline(&s, &t, fp) > 0)
			++count;
		free(s);
		fclose(fp);
	}
	return count;
}

////////// Main ///////////////////////////////////////////////////////////////

int main(void)
{
	int len;

	if ((len = size()) > 0)
	{
		printf("%d\n", len);
	}
	return 0;
}
