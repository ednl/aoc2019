///////////////////////////////////////////////////////////////////////////////
//
// Advent of Code 2019
// Day 1b: rocket fuel for modules + fuel
//
// E. Dronkert
// https://github.com/ednl/aoc2019
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>   // fopen, fgets, printf
#include <stdlib.h>  // atoi

#define BUFLEN 16

const char *inp = "inp01.txt";

int fuel(int mass)
{
	if ((mass = mass / 3 - 2) > 0)
		return mass;
	return 0;
}

int main(void)
{
	FILE *fp;
	char line[BUFLEN];
	int i, a = 0;

	if ((fp = fopen(inp, "r")) != NULL)
		while (fgets(line, sizeof line, fp) != NULL) {
			i = atoi(line);
			while ((i = fuel(i)) > 0)
				a += i;
		}

	printf("Fuel: %u\n", a);
	return 0;
}
