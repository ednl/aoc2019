///////////////////////////////////////////////////////////////////////////////
// Advent of Code 2019
// Day 14:
//
// E. Dronkert
// https://github.com/ednl/aoc2019
///////////////////////////////////////////////////////////////////////////////

////////// Includes & Defines /////////////////////////////////////////////////

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>   // fopen, fgetc, getdelim, printf
#include <stdlib.h>  // atoi, atol

#define DEBUG

#define MAXEQ 100  // maximum number of equations = unique elements
#define MAXELM 10  // maximum number of elements produced by 1 reverse equation
#define MAXNAME 7  // maximum number of letters in element name

////////// Typedefs & Constants ///////////////////////////////////////////////

// Puzzle input
//static const char *inp = "test.txt";
static const char *inp = "inp14.txt";

// Element, quantity
typedef struct Elm {
	int id;  // ID from element name
	int q;   // quantity
} ELM, *PELM;

// Reverse equation
typedef struct RevEq {
	int len; // length of equation = unique elements it produces
	ELM inp; // origin element & quantity
	ELM out[MAXELM]; // elements & quantities produced by this equation
} REVEQ, *PREVEQ;

////////// Globals ////////////////////////////////////////////////////////////

REVEQ eq[MAXEQ];  // reverse equations
int neq = 0;      // number of reverse equations
ELM prod[MAXEQ];  // quantities produced

int oreid = 0, fuelid = 0;

////////// Function Declarations //////////////////////////////////////////////

int elmid(char *);
int elmname(int);
ELM parseelm(char *s);
int readfile(void);

////////// Function Definitions ///////////////////////////////////////////////

int elmid(char *s)
{
	static const int e0 = 'A' - 1;  // A=1, zero=A-1
	static const int en = 'Z';      // Z=26
	static const int base = en - e0 + 1;  // range=0..26=27
	int id = 0;
	char c, *pc;

	pc = s;
	while ((c = *pc++) && c > e0 && c <= en)  // stop at '\0' or out of range
		id = id * base + c - e0;
	return id;
}

int elmname(int id)
{
	static const int e0 = 'A' - 1;  // A=1, zero=A-1
	static const int base = 'Z' - e0 + 1;  // range=0..26=27
	char s[MAXNAME + 1];  // name & '\0'
	int i, n = 0;

	while (id && n < MAXNAME)
	{
		s[n++] = e0 + id % base;
		id /= base;
	}
	s[n] = '\0';
	for (i = n - 1; i >= 0; --i)
		printf("%c", s[i]);
	return n;
}

ELM parseelm(char *s)
{
	int id = 0, q = 0;
	char c, *pc;

	pc = s;
	while ((c = *pc) && (c < '0' || c > '9'))
		pc++;
	if (c)
	{
		q = atoi(pc);
		while ((c = *pc) && (c < 'A' || c > 'Z'))
			pc++;
		if (c)
			id = elmid(pc);
	}
	return (ELM){ id, q };
}

int readfile(void)
{
	FILE *fp;        // file pointer
	char *s = NULL;  // dynamically allocated buffer
	size_t t = 0;    // size of buffer
	char c, *pc;
	int i, line = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		while (getline(&s, &t, fp) > 0)
		{
			pc = s;  // start at beginning of line
			i = 0;   // element index
			while ((c = *pc) && c != '\r' && c != '\n')
			{
				eq[line].out[i++] = parseelm(pc);
				while ((c = *pc) && c != ',' && c != '>')
					pc++;
				if (c)
					pc++;  // skip the divider but not '\0'
				if (c == '>')
				{
					eq[line].inp = parseelm(pc);
					break;
				}
			}
			eq[line++].len = i;
		}
		free(s);
		fclose(fp);
	}
	return line;
}

int cmpelm(const void *a, const void *b)
{
	if (((PELM)a)->id < ((PELM)b)->id)
		return -1;
	if (((PELM)a)->id > ((PELM)b)->id)
		return 1;
	return 0;
}

int cmpeq(const void *a, const void *b)
{
	if (((PREVEQ)a)->inp.id == ((PREVEQ)b)->inp.id)
		return 0;
	if (((PREVEQ)a)->inp.id == fuelid)
		return -1;
	if (((PREVEQ)b)->inp.id == fuelid)
		return 1;
	if (((PREVEQ)a)->inp.id < ((PREVEQ)b)->inp.id)
		return -1;
	return 1;
}

////////// Main ///////////////////////////////////////////////////////////////

int main(void)
{
	int neq, i, j;

	// Init
	for (i = 0; i < MAXEQ; ++i)
	{
		eq[i].len = MAXELM;
		eq[i].inp = (ELM){ 0, 0 };
		for (j = 0; j < MAXELM; ++j)
			eq[i].out[j] = (ELM){ 0, 0 };
		prod[i] = (ELM){ 0, 0 };
	}

	oreid = elmid("ORE");
	fuelid = elmid("FUEL");

	// Parse
	if ((neq = readfile()))
	{
		qsort(eq, neq, sizeof(REVEQ), cmpeq);
		for (i = 0; i < neq; ++i)
			qsort(eq[i].out, eq[i].len, sizeof(ELM), cmpelm);

		// Print reverse equations
		for (i = 0; i < neq; ++i)
		{
			printf("%d ", eq[i].inp.q);
			elmname(eq[i].inp.id);
			printf(" =>");
			for (j = 0; j < eq[i].len; ++j)
			{
				if (j)
					printf(",");
				printf(" %d ", eq[i].out[j].q);
				elmname(eq[i].out[j].id);
			}
			printf("\n");
		}

		// Print production
		for (i = 0; i < neq; ++i)
		{
		}
	}

	return 0;
}
