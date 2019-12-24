///////////////////////////////////////////////////////////////////////////////
////
////  Advent of Code 2019
////  Day 24: Planet of Discord, part two
////
////  E. Dronkert
////  https://github.com/ednl/aoc2019
////
///////////////////////////////////////////////////////////////////////////////

////////// Includes & Defines /////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#define DIM 5
#define STEPS 200

#define AREA (DIM * DIM)       // number of tiles in one level
#define LEVELS (STEPS + 1)     // maximum number of levels (positive + zero + negative)
#define TILES (LEVELS * AREA)  // total number of tiles
#define LEVELMAX (STEPS / 2)
#define LEVELMIN (-LEVELMAX)

////////// Function Definitions ///////////////////////////////////////////////

int ix(int level, int x, int y)
{
	return (level - LEVELMIN) * AREA + y * DIM + x;
}

void printlevels(int *a)
{
	int n, m, x, y;
	
	for (m = LEVELMIN; m <= LEVELMAX; m += 26)
	{
		for (y = 0; y < DIM; ++y)
		{
			for (n = m; n < m + 26 && n <= LEVELMAX; ++n)
			{
				for (x = 0; x < DIM; ++x)
				{
					if (x == 2 && y == 2)
						printf("?");
					else
						printf("%c", a[ix(n, x, y)] ? '#' : '.');
				}
				printf(" ");
			}
			printf("\n");
		}
		printf("\n");
	}
}

int bugrow(int *a, int level, int row)
{
	int x, sum = 0;

	if (level > LEVELMAX)
		return 0;  // out of range

	for (x = 0; x < DIM; ++x)
		sum += a[ix(level, x, row)];

	return sum;
}

int bugcol(int *a, int level, int col)
{
	int y, sum = 0;

	if (level > LEVELMAX)
		return 0;  // out of range

	for (y = 0; y < DIM; ++y)
		sum += a[ix(level, col, y)];

	return sum;
}

int bug(int *a, int level, int x, int y)
{
	if (level < LEVELMIN)
		return 0;  // out of range

	if (x == -1)
		return bug(a, level - 1, 1, 2);
	if (x == DIM)
		return bug(a, level - 1, 3, 2);
	if (y == -1)
		return bug(a, level - 1, 2, 1);
	if (y >= DIM)
		return bug(a, level - 1, 2, 3);

	return a[ix(level, x, y)];
}

int neighbours(int *a, int level, int x, int y)
{
	int x1, y1, sum = 0;

	if (level < LEVELMIN || level > LEVELMAX)
		return 0;  // out of range

	x1 = x; y1 = y + 1;  // below
	sum += x1 == 2 && y1 == 2 ? bugrow(a, level + 1, 0)       : bug(a, level, x1, y1);

	x1 = x; y1 = y - 1;  // above
	sum += x1 == 2 && y1 == 2 ? bugrow(a, level + 1, DIM - 1) : bug(a, level, x1, y1);

	x1 = x + 1; y1 = y;  // right
	sum += x1 == 2 && y1 == 2 ? bugcol(a, level + 1, 0)       : bug(a, level, x1, y1);

	x1 = x - 1; y1 = y;  // left
	sum += x1 == 2 && y1 == 2 ? bugcol(a, level + 1, DIM - 1) : bug(a, level, x1, y1);

	return sum;
}

void evolve(int *cur, int *nxt, int lim)
{
	int i, x, y, n, j = ix(-lim, 0, 0);

	for (i = -lim; i <= lim; ++i)
		for (y = 0; y < DIM; ++y)
			for (x = 0; x < DIM; ++x)
			{
				if (x != 2 || y != 2)
				{
					n = neighbours(cur, i, x, y);
					nxt[j] = cur[j] ? n == 1 : n == 1 || n == 2;
				}
				++j;
			}
}

////////// Main ///////////////////////////////////////////////////////////////

int main(void)
{
	FILE *fp;
	int c, i, j, x, y, sum;
	int grid[TILES], next[TILES];
	int *a, *b, *t;

	for (i = 0; i < TILES; ++i)
		grid[i] = 0;

	if ((fp = fopen("24.txt", "r")) != NULL)
	{
		if ((i = ix(0, 0, 0)) >= 0)
			while ((c = fgetc(fp)) != EOF)
				if (c == '#' || c == '.' || c == '?')
					grid[i++] = c == '#';
		fclose(fp);
	}

	a = grid;
	b = next;
	// printf("\033[?25l");  // hide cursor
	// printf("\033[2J\033[1;1H");  // cls-home
	// printf("gen   0\n\n");
	// printlevels(a);
	for (i = 0; i < STEPS; ++i)
	{
		evolve(a, b, i / 2 + 1);
		t = a;
		a = b;
		b = t;
		// printf("\033[1;1H");  // home
		// printf("gen %3d\n\n", i + 1);
		// printlevels(a);
		// for (j = 0; j < 10000000; ++j)
		// 	sum += (sum + j) % (j + 123456789);  // trick compiler
	}
	// printf("\033[?25h");  // show cursor

	sum = 0;
	j = 0;
	for (i = LEVELMIN; i <= LEVELMAX; ++i)
		for (y = 0; y < DIM; ++y)
			for (x = 0; x < DIM; ++x)
			{
				if (x != 2 || y != 2)
					sum += a[j];
				++j;
			}
	printf("%u\n", sum);

	return 0;
}
