/******************************************************************************
 * Advent of Code 2019
 * Day 1a: rocket fuel for modules
 *
 * E. Dronkert
 * https://github.com/ednl/aoc2019
 *****************************************************************************/

#include <stdio.h>   // fopen, fgets, printf
#include <stdlib.h>  // atoi

#define BUFLEN 16

int main(void)
{
	FILE *fp;
	char line[BUFLEN];
	int i, a = 0;

	if ((fp = fopen("inp01a.txt", "r")) != NULL)
		while (fgets(line, sizeof line, fp) != NULL)
			if ((i = atoi(line) / 3 - 2) > 0)
				a += i;

	printf("Fuel: %u\n", a);
	return 0;
}
