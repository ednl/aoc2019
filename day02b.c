///////////////////////////////////////////////////////////////////////////////
//
// Advent of Code 2019
// Day 2b: Intcode program + target output
//
// E. Dronkert
// https://github.com/ednl/aoc2019
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>   // fopen, fgetc, printf
#include <stdlib.h>  // atoi

#define BUFLEN 16

#define ADD  1
#define MUL  2
#define RET 99

#define TARGET 19690720

void copy(int *src, int *dst, int len)
{
	while (len--)
		*dst++ = *src++;
}

int main(void)
{
	FILE *fp;
	char val[BUFLEN];
	int i, c;

	int *data, *memory, noun, verb, ip, op, np, size = 1;
	int p1, p2, p3;

	// Count fields
	if ((fp = fopen("inp02a.txt", "r")) != NULL)
		while ((c = fgetc(fp)) != EOF)
			if (c == ',')
				++size;

	// Reserve space
	data  = (int *)malloc(size * sizeof(int));
	memory = (int *)malloc(size * sizeof(int));

	if (data == NULL || memory == NULL) {
		puts("Out of memory");
		return 1;
	}

	// Read data
	rewind(fp);
	i = 0;
	ip = 0;
	while ((c = fgetc(fp)) != EOF)
		if (c >= '0' && c <= '9' && i < BUFLEN - 1)
			val[i++] = c;
		else {
			if (i > 0 && ip < size) {
				val[i] = '\0';
				data[ip++] = atoi(val);
			}
			i = 0;
		}
	if (i > 0 && ip < size) {
		val[i] = '\0';
		data[ip++] = atoi(val);
	}

	// Seek target
	noun = 0;
	verb = 0;
	do {

		// Initialise program
		copy(data, memory, size);
		memory[1] = noun;
		memory[2] = verb;

		// Run program
		ip = 0;
		while (ip < size && (op = memory[ip]) != RET) {
			if (op == ADD || op == MUL)
				np = 3;
			else
				np = 0;
			if (ip + np < size) {
				if (np == 3) {
					p1 = memory[ip + 1];
					p2 = memory[ip + 2];
					p3 = memory[ip + 3];
					if (p1 >= 0 && p2 >= 0 && p3 >= 0 &&
						p1 < size && p2 < size && p3 < size)
						switch (op) {
							case ADD: memory[p3] = memory[p1] + memory[p2]; break;
							case MUL: memory[p3] = memory[p1] * memory[p2]; break;
						}
				}
			}
			ip += np + 1;
		}

		// Check target
		if (memory[0] == TARGET)
			break;

		// Next iteration
		++verb;
		if (verb > RET) {
			verb = 0;
			++noun;
		}

	} while (noun <= RET);

	printf("%i\n", noun * 100 + verb);
	free(data);
	free(memory);
	return 0;
}
