///////////////////////////////////////////////////////////////////////////////
// Advent of Code 2019
// Day 16: Flawed Frequency Transmission, part one
//
// E. Dronkert
// https://github.com/ednl/aoc2019
///////////////////////////////////////////////////////////////////////////////

// Each element in the new list is built by multiplying every value in the input
// list by a value in a repeating pattern and then adding up the results. So, if
// the input list were 9, 8, 7, 6, 5 and the pattern for a given element were 1,
// 2, 3, the result would be 9*1 + 8*2 + 7*3 + 6*1 + 5*2 (with each input element
// on the left and each value in the repeating pattern on the right of each
// multiplication). Then, only the ones digit is kept: 38 becomes 8, -17 becomes
// 7, and so on.

// While each element in the output array uses all of the same input array
// elements, the actual repeating pattern to use depends on which output element
// is being calculated. The base pattern is 0, 1, 0, -1. Then, repeat each value
// in the pattern a number of times equal to the position in the output list
// being considered. Repeat once for the first element, twice for the second
// element, three times for the third element, and so on. So, if the third
// element of the output list is being calculated, repeating the values would
// produce: 0, 0, 0, 1, 1, 1, 0, 0, 0, -1, -1, -1.

// When applying the pattern, skip the very first value exactly once. (In other
// words, offset the whole pattern left by one.) So, for the second element of
// the output list, the actual pattern used would be: 0, 1, 1, 0, 0, -1, -1, 0, 0,
// 1, 1, 0, 0, -1, -1, ....

// After using this process to calculate each element of the output list, the
// phase is complete, and the output list of this phase is used as the new input
// list for the next phase, if any.

// Here are the first eight digits of the final output list after 100 phases for
// some larger inputs:

//   80871224585914546619083218645595 becomes 24176176.
//   19617804207202209144916044189917 becomes 73745418.
//   69317163492948606335995924319873 becomes 52432133.

// After 100 phases of FFT, what are the first eight digits in the final output
// list?

////////// Includes & Defines /////////////////////////////////////////////////

#include <stdio.h>   // fopen, fgetc, getdelim, printf
#include <stdlib.h>  // atoi, atol
#include <ctype.h>   // isdigit

#define LEN 1000

const static int pat[] = { 0, 1, 0, -1 };
static int a[LEN];
static int b[LEN];
static int *inp, *out;

int main(void)
{
	FILE *fp;
	int i, c, len;

	inp = a;
	out = b;
	if ((fp = fopen("inp16.txt", "r")) != NULL)
	{
		i = 0;
		while (i < LEN && (c = fgetc(fp)) != EOF)
			if (isdigit(c))
				inp[i++] = c - '0';
	}
	len = i;

	return 0;
}
