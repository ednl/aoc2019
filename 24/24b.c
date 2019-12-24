#include <stdio.h>
#include <stdlib.h>

#define DIM 5
#define STEPS 200

#define AREA (DIM * DIM)       // number of tiles in one level
#define LEVELS (STEPS + 1)     // maximum number of levels (positive + zero + negative)
#define TILES (LEVELS * AREA)  // total number of tiles
#define LEVELMAX (STEPS / 2)
#define LEVELMIN (-LEVELMAX)

int grid[TILES], next[TILES];

int ix(int level, int x, int y)
{
	if (level < LEVELMIN || level > LEVELMAX || x < 0 || x >= DIM || y < 0 || y >= DIM)
		return -1;  // invalid

	return (level - LEVELMIN) * AREA + y * DIM + x;
}

void printlevels(int *a)
{
	int n, x, y, i, j;
	
	for (j = LEVELMIN; j <= LEVELMAX; j += 39)
	{
		for (y = 0; y < DIM; ++y)
		{
			for (n = j; n < j + 39 && n <= LEVELMAX; ++n)
			{
				for (x = 0; x < DIM; ++x)
				{
					if (x == 2 && y == 2)
						printf(" ");
					else if ((i = ix(n, x, y)) >= 0)
						printf("%c", a[i] ? '#' : '.');
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
	int x, i, sum = 0;

	if (level < LEVELMIN || level > LEVELMAX || row < 0 || row >= DIM)
		return 0;  // invalid

	for (x = 0; x < DIM; ++x)
		if ((i = ix(level, x, row)) >= 0)
			sum += a[i];

	return sum;
}

int bugcol(int *a, int level, int col)
{
	int y, i, sum = 0;

	if (level < LEVELMIN || level > LEVELMAX || col < 0 || col >= DIM)
		return 0;  // invalid

	for (y = 0; y < DIM; ++y)
		if ((i = ix(level, col, y)) >= 0)
			sum += a[i];

	return sum;
}

int bug(int *a, int level, int x, int y)
{
	int i;

	if ((x == 2 && y == 2) || x < -1 || x > DIM || y < -1 || y > DIM)
	{
		printf("Error in bug: lev=%d x=%d y=%d\n", level, x, y);
		exit(1);
	}

	if (level < LEVELMIN || level > LEVELMAX)
		return 0;  // invalid

	// outside level
	if (x == -1)
		return bug(a, level - 1, 1, 2);
	if (x == DIM)
		return bug(a, level - 1, 3, 2);
	if (y == -1)
		return bug(a, level - 1, 2, 1);
	if (y >= DIM)
		return bug(a, level - 1, 2, 3);

	// normal tile
	if ((i = ix(level, x, y)) >= 0)
		return a[i];
	
	printf("Error: lev=%d x=%d y=%d i=%d\n", level, x, y, i);
	exit(1);
}

int neighbours(int *a, int level, int x, int y)
{
	int x1, y1, sum = 0;

	if ((x == 2 && y == 2) || x < -1 || x > DIM || y < -1 || y > DIM)
	{
		printf("Error in neighbours: lev=%d x=%d y=%d\n", level, x, y);
		exit(1);
	}

	if (level < LEVELMIN || level > LEVELMAX)
		return 0;  // invalid

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

void evolve(int *cur, int *nxt)
{
	int i, j, x, y, n;

	for (i = LEVELMIN; i <= LEVELMAX; ++i)
		for (y = 0; y < DIM; ++y)
			for (x = 0; x < DIM; ++x)
				if ((x != 2 || y != 2) && (j = ix(i, x, y)) >= 0)
				{
					n = neighbours(cur, i, x, y);
					nxt[j] = cur[j] ? n == 1 : n == 1 || n == 2;
				}
}

int main(void)
{
	FILE *fp;
	int c, i, j, x, y, sum;
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
	printf("\033[?25l");
	printf("\033[2J\033[1;1H");
	printf("gen   0\n\n");
	printlevels(a);
	for (i = 0; i < STEPS; ++i)
	{
		evolve(a, b);
		t = a;
		a = b;
		b = t;
		printf("\033[1;1H");
		printf("gen %3d\n\n", i + 1);
		printlevels(a);
		for (j = 0; j < 20000000; ++j)
			sum += (sum + j) % (j + 123456789);
	}

	sum = 0;
	for (i = LEVELMIN; i <= LEVELMAX; ++i)
		for (y = 0; y < DIM; ++y)
			for (x = 0; x < DIM; ++x)
				if ((x != 2 || y != 2) && (j = ix(i, x, y)) >= 0)
					sum += a[j];
	printf("sum %u\n", sum);
	printf("\033[?25h");

	return 0;
}
