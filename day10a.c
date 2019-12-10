///////////////////////////////////////////////////////////////////////////////
//
// Advent of Code 2019
// Day 10: Monitoring Station, part one
//
// E. Dronkert
// https://github.com/ednl/aoc2019
//
///////////////////////////////////////////////////////////////////////////////

////////// Includes & Defines /////////////////////////////////////////////////

#include <stdio.h>   // fopen, fgetc, printf
#include <stdlib.h>  // atoi, atol

////////// Typedefs & Constants ///////////////////////////////////////////////

typedef struct Pos {
	int x, y, n;
} POS, *PPOS;

// Location of program on disk
//const char *inp = "test.txt";
const char *inp = "inp10.txt";

////////// Globals ////////////////////////////////////////////////////////////

PPOS asteroids = NULL;
int *field = NULL;
int width, depth, count;

////////// Functions //////////////////////////////////////////////////////////

// Count hashes in file
int size(void)
{
	FILE *fp;
	int c, n = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		while ((c = fgetc(fp)) != EOF)
			if (c == '#')
				++n;
		fclose(fp);
	}
	return n;
}

// Count lines in file
int lines(void)
{
	FILE *fp;
	char *s = NULL;
	size_t t = 0;
	int n = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		while (getline(&s, &t, fp) > 0)
			++n;
		free(s);
		fclose(fp);
	}
	return n;
}

// Count characters in first line of file
int chars(void)
{
	FILE *fp;
	char *s = NULL;
	size_t t = 0;
	int n = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		if ((n = getline(&s, &t, fp)) > 0)
			while (n && (s[n - 1] == '\n' || s[n - 1] == '\r'))
				n--;
		free(s);
		fclose(fp);
	}
	return n;
}

// Read hash positions from file to memory
// Arg: a must be allocated
int read(void)
{
	FILE *fp;
	int c, i = 0, p = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		while ((c = fgetc(fp)) != EOF)
			if (c == '#' || c == '.')
			{
				if (c == '#')
					asteroids[p++] = (POS){ i % width, i / width, 0 };
				field[i++] = (c == '#');
			}
		fclose(fp);
	}
	return p;
}

int cansee(int x, int y, int sx, int sy, int sn)
{
	int i, test = 1;

	for (i = 1; i < sn; ++i)  // number of steps to integer intermediate points
	{
		x += sx;  // next intermediate point
		y += sy;
		if (field[y * width + x])  // asteroid in the way?
		{
			test = 0;  // we can't see asteroid j from i
			break;     // no point in looking further
		}
	}
	return test;
}

////////// Main ///////////////////////////////////////////////////////////////

int main(void)
{
	int i, j, k, dx, dy, s, t, stepx, stepy, steps, max = 0, imax = 0, min;

	width = chars();
	depth = lines();
	count = size();
	//printf("w=%d h=%d n=%d\n", width, depth, count);
	if (width > 0 && depth > 0 && count > 0)
	{
		asteroids = malloc(count * sizeof *asteroids);
		field = malloc(width * depth * sizeof *field);
		if (asteroids != NULL && field != NULL)
		{
			if (read() == count)
			{
				k = 0;
				for (i = 0; i < depth; ++i)
				{
					for (j = 0; j < width; ++j)
						printf("%c", field[k++] ? '#' : '.');
					printf("\n");
				}
				// Data is good
				for (i = 0; i < count; ++i)  // test every asteroid
				{
					//printf("from [i=%d x=%d y=%d]\n", i, asteroids[i].x, asteroids[i].y);
					for (j = 0; j < count; ++j)
					{
						if (i != j)  // compare with every other asteroid
						{
							// get common factors from dx, dy
							dx = asteroids[j].x - asteroids[i].x;  // from i to j
							dy = asteroids[j].y - asteroids[i].y;
							//printf("to [j=%d x=%d y=%d] [dx=%d dy=%d] ", j, asteroids[j].x, asteroids[j].y, dx, dy);
							if (abs(dx) > 1 || abs(dy) > 1)
							{
								if (!dx || !dy)
								{
									// horizontal or vertical
									stepx = dx ? dx / abs(dx) : 0;
									stepy = dy ? dy / abs(dy) : 0;
									steps = dx ? abs(dx) : abs(dy);
									t = cansee(asteroids[i].x, asteroids[i].y, stepx, stepy, steps);
									asteroids[i].n += t;
									//printf("sx=%d sy=%d sn=%d %s\n", stepx, stepy, steps, t ? "Y0" : "n0");
								} else if (abs(dx) == abs(dy))
								{
									// diagonal
									stepx = dx / abs(dx);
									stepy = dy / abs(dy);
									steps = abs(dx);
									t = cansee(asteroids[i].x, asteroids[i].y, stepx, stepy, steps);
									asteroids[i].n += t;
									//printf("sx=%d sy=%d sn=%d %s\n", stepx, stepy, steps, t ? "Y1" : "n1");
								} else
								{
									// at an angle
									t = 1;  // premise is: yes, we see j from i
									min = abs(dx) < abs(dy) ? abs(dx) : abs(dy);
									for (k = 2; k <= min; ++k)
									{
										if (!(dx % k) && !(dy % k))  // must be integer steps
										{
											stepx = dx / k;  // integer steps from asteroid i to j
											stepy = dy / k;
											steps = k;
											//printf("sx=%d sy=%d sn=%d ", stepx, stepy, steps);
											t = t && cansee(asteroids[i].x, asteroids[i].y, stepx, stepy, steps);
											if (!t)
												break;
										}
									}
									//printf("%s\n", t ? "Yk" : "nk");
									if (t)         // no intermediate asteroid?
										asteroids[i].n++;  // asteroid j can be seen from i
								}
							} else
							{
								// too close, no intermediate points
								//printf("Y\n");
								asteroids[i].n++;
							}
						}
					}
					//printf("%2d (%d,%d) = %2d\n", i, asteroids[i].x, asteroids[i].y, asteroids[i].n);
					if (asteroids[i].n > max)
					{
						max = asteroids[i].n;
						imax = i;
					}
				}
			}
		}
		free(field);
		free(asteroids);
	}
	printf("max %d(%d,%d) = %d\n", imax, asteroids[imax].x, asteroids[imax].y, max);
	return 0;
}
