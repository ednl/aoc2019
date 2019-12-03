/******************************************************************************
 * Advent of Code 2019
 * Day 1: rocket fuel
 *
 * E. Dronkert
 * https://github.com/ednl/aoc2019
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define BUFLEN 16

int main(void)
{
	FILE *fp;
	char line[BUFLEN];
	unsigned int i, n = 0, m = 0, a = 0;

	fp = fopen("input1.txt", "r");
	if (fp != NULL) {
		while (fgets(line, sizeof line, fp) != NULL) {
			++n;
			i = atoi(line);
			if (i >= 6) {
				a += i / 3 - 2;
				++m;
			}
		}
	}
	printf("Lines   : %u\n", n);
	printf("Modules : %u\n", m);
	printf("Fuel    : %u\n", a);
	return 0;
}
