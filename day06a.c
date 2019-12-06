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

#define YOU (256 * (256 * 'Y' + 'O') + 'U')
#define SAN (256 * (256 * 'S' + 'A') + 'N')

////////// Typedefs & Constants ///////////////////////////////////////////////

typedef struct Orbit {
	int parent, around, inorbit;
} ORBIT, *PORBIT;

const char *inp = "inp06.txt";

////////// Globals ////////////////////////////////////////////////////////////

int you = -1, san = -1;

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

// Read orbit lines from file to memory
// Arg: array a must be allocated
int read(PORBIT a, int n)
{
	FILE *fp;
	char *s = NULL;
	size_t t = 0;
	int i, ar, in, line = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		while (line < n && getline(&s, &t, fp) > 0)
		{
			i = 0;
			ar = 0;
			in = 0;
			while (i < 3 && i < t)
				ar = ar * 256 + s[i++];
			if (i == 3)
				++i;                     // skip ')'
			while (i < 7 && i < t)
				in = in * 256 + s[i++];
			if (in == YOU)
				you = line;
			else if (in == SAN)
				san = line;
			a[line++] = (ORBIT){ -1, ar, in };
		}
		free(s);
		fclose(fp);
	}
	return line;
}

void findparent(PORBIT a, int n)
{
	int i, j;

	for (i = 0; i < n; ++i)
		for (j = 0; j < n; ++j)
			if (a[j].inorbit == a[i].around)
			{
				a[i].parent = j;
				break;
			}
}

int countorbits(PORBIT a, int n)
{
	int i, j, k, count = 0;

	for (i = 0; i < n; ++i)
	{
		j = i;
		k = 0;
		while ((j = a[j].parent) >= 0)
			++k;
		count += k;
	}
	return count;
}

////////// Main ///////////////////////////////////////////////////////////////

int main(void)
{
	int len;
	PORBIT orb = NULL;

	if ((len = size()) > 0)
	{
		if ((orb = malloc(len * sizeof *orb)) != NULL)
		{
			if (read(orb, len) == len)
			{
				findparent(orb, len);
				printf("%d\n", countorbits(orb, len));
			}
			free(orb);
		}
	}
	return 0;
}
