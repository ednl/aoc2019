///////////////////////////////////////////////////////////////////////////////
////
////  Advent of Code 2019
////  Day 17: Set and Forget, part one
////
////  E. Dronkert
////  https://github.com/ednl/aoc2019
////
///////////////////////////////////////////////////////////////////////////////

////////// Includes & Defines /////////////////////////////////////////////////

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>   // fopen, fgetc, getdelim, printf
#include <stdlib.h>  // atoi, atol
#include <ctype.h>   // isdigit
#include <time.h>    // time for srand
#include <string.h>  // strcpy

#define DEBUG

// Virtual machine operation
#define VM_FNLEN  32  // max file name length of program on disk
#define VM_PAD  3000  // storage space to add at end of program
#define VM_RESUME  0
#define VM_RESET   1

// Error codes
#define ERR_OK               0
#define ERR_OPCODE_UNKNOWN   1
#define ERR_OPCODE_UNDEFINED 2
#define ERR_SEGFAULT_PARAM   3
#define ERR_SEGFAULT_READ    4
#define ERR_SEGFAULT_WRITE   5
#define ERR_SEGFAULT_OTHER   6

// CPU opcodes
#define CPU_ADD  1L  // add
#define CPU_MUL  2L  // multiply
#define CPU_IN   3L  // get input
#define CPU_OUT  4L  // send output
#define CPU_JNZ  5L  // jump non-zero
#define CPU_JZ   6L  // jump zero
#define CPU_LT   7L  // compare less than
#define CPU_EQ   8L  // compare equal
#define CPU_RBO  9L  // relative base offset
#define CPU_HLT 99L  // return (halt)

// Opcode parameters
#define PAR_POS 0L  // positional
#define PAR_IMM 1L  // immediate
#define PAR_OFF 2L  // offset
#define PAR_MAX 3L  // max number of parameters per opcode

// Game
#define MAZE_NODIR  0
#define MAZE_NORTH  1
#define MAZE_SOUTH  2
#define MAZE_WEST   3
#define MAZE_EAST   4
#define MAZE_VOID  -1
#define MAZE_WALL   0
#define MAZE_FREE   1
#define MAZE_DEST   2
#define MAZE_DIMX  41
#define MAZE_DIMY  60

////////// Typedefs & Constants ///////////////////////////////////////////////

// Language entries (instruction definition)
typedef struct Instr {
	int opcode;
	int read, write;
} INSTR;

// Language definition: { opcode, read par count, write par count }
// Every read parameter can be positional or immediate
// Write parameters are always positional
static const INSTR cpu[] = {
	{ CPU_ADD, 2, 1 },
	{ CPU_MUL, 2, 1 },
	{ CPU_IN , 0, 1 },
	{ CPU_OUT, 1, 0 },
	{ CPU_JNZ, 2, 0 },
	{ CPU_JZ , 2, 0 },
	{ CPU_LT , 2, 1 },
	{ CPU_EQ , 2, 1 },
	{ CPU_RBO, 1, 0 },
	{ CPU_HLT, 0, 0 }
};
static const int cpusize = sizeof cpu / sizeof *cpu;

static const char *maze_sol = "R,12,L,10,L,10,L,6,L,12,R,12,L,4,R,12,L,10,L,10,L,6,L,12,";

////////// Globals ////////////////////////////////////////////////////////////

// Virtual machine
static char vm_filename[VM_FNLEN + 1];
static long *vm_cache = NULL, *vm_memory = NULL;
static int vm_cachesize = 0, vm_memorysize = 0;

// Puzzle 17
static int orientation = MAZE_NODIR, steps = 0;
static int droidx = -1, droidy = -1;
static int maze[MAZE_DIMX * MAZE_DIMY];

////////// Function Declarations //////////////////////////////////////////////

int aoc_thispuzzle(char **);
void aoc_setfilename(int);
int aoc_csvfields(void);
int aoc_csv2cache(void);
void vm_refresh(void);
int vm_exec(int);
long vm_input(void);
void vm_output(long);

// Maze functions
char maze_dir2char(int);
int maze_char2dir(char);
int maze_index(int, int);
int maze_peek(int, int, int);
int maze_alignment(int, int);
int maze_calibration(void);
int maze_deadend(int, int);
// int maze_reverse(int);
// void maze_draw(void);

////////// Function Definitions ///////////////////////////////////////////////

int aoc_thispuzzle(char **exe)
{
	char *pc;
	int c;

	pc = *exe;
	while ((c = *pc) && !isdigit(c))
		++pc;
	return atoi(pc);
}

void aoc_setfilename(int num)
{
	static const char *test = "test.txt";
	static const char *fmt = "%d.txt";

	if (num)
		snprintf(vm_filename, VM_FNLEN, fmt, num);
	else
		strcpy(vm_filename, test);
}

// Count values in a one-line CSV file by counting commas + 1
int aoc_csvfields(void)
{
	FILE *fp;
	int c, i = 0;

	if ((fp = fopen(vm_filename, "r")) != NULL)
	{
		i = 1;
		while ((c = fgetc(fp)) != EOF)
			if (c == ',')
				++i;
		fclose(fp);
	}
	return i;
}

// Read long integer CSV values from file to cache
// Pre: vm_cache must be allocated to size vm_cachesize > 0
//      vm_filename must contain name of readable file
// Ret: number of values read
int aoc_csv2cache(void)
{
	FILE *fp;        // file pointer
	char *s = NULL;  // dynamically allocated buffer
	size_t t = 0;    // size of buffer
	int i = 0;       // values read

	if (vm_cache != NULL && vm_cachesize > 0 &&
		(fp = fopen(vm_filename, "r")) != NULL)
	{
		while (i < vm_cachesize && getdelim(&s, &t, ',', fp) > 0)
			vm_cache[i++] = atol(s);
		free(s);
		fclose(fp);
	}
	return i;
}

// Copy program from cache to memory, and zero the padding
// Pre: dat must be allocated to size datsize > 0
//      mem must be allocated to size memsize >= datsize
void vm_refresh(void)
{
	long *src, *dst;
	int i;

	if (vm_cache != NULL && vm_memory != NULL &&
		vm_cachesize > 0 && vm_cachesize <= vm_memorysize)
	{
		src = vm_cache;
		dst = vm_memory;
		i = vm_cachesize;
		while (i--)
			*dst++ = *src++;
		i = vm_memorysize - vm_cachesize;
		while (i--)
			*dst++ = 0;
	}
}

// Parse and execute all instructions in memory
int vm_exec(int reset)
{
	static int ip = 0;    // instruction pointer
	static int base = 0;  // "relative base offset"
	static int tick = 0;
	int instr, opcode, parmode;
	long par[PAR_MAX];
	int i, j, k;

	if (reset)
	{
		vm_refresh();
		ip = 0;
		base = 0;
	}

	while (ip >= 0 && ip < vm_memorysize)
	{
		// Get opcode and parameter modes from instruction
		instr = vm_memory[ip++];       // get instruction, increment instr pointer
		opcode = instr % 100;          // opcode part of instruction
		instr /= 100;                  // this leaves parameter modes

		// Look up opcode, halt if not found or too long
		i = 0;
		while (i < cpusize && opcode != cpu[i].opcode)
			++i;
		if (i == cpusize)             // not in the language def?
		{
			#ifdef DEBUG
			printf("Unknown opcode %d at %d\n", opcode, ip - 1);
			#endif
			return ERR_OPCODE_UNKNOWN;
		}
		if (ip + cpu[i].read + cpu[i].write > vm_memorysize)
		{
			#ifdef DEBUG
			printf("Too many parameters for program size at %d\n", ip - 1);
			#endif
			return ERR_SEGFAULT_PARAM;
		}

		// Get "read" parameter(s)
		for (j = 0; j < cpu[i].read; ++j)
		{
			par[j] = vm_memory[ip++];  // get immediate value, increment instr pointer
			parmode = instr % 10;
			instr /= 10;               // next parameter mode
			if (parmode != PAR_IMM)    // par mode = positional or offset?
			{
				if (parmode == PAR_OFF)  // use base offset
					par[j] += base;
				if (par[j] < 0 || par[j] >= vm_memorysize)
				{
					#ifdef DEBUG
					printf("Read beyond program size: %ld\n", par[j] - vm_memorysize + 1);
					#endif
					return ERR_SEGFAULT_READ;
				}
				par[j] = vm_memory[par[j]];  // get positional value
			}
		}

		// Get "write" parameter (always positional/offset but keep the address this time)
		if (cpu[i].write)
		{
			par[j] = vm_memory[ip++];   // get address param, increment program counter
			if (instr % 10 == PAR_OFF)  // par mode = offset?
				par[j] += base;
			if (par[j] < 0 || par[j] >= vm_memorysize)
			{
				#ifdef DEBUG
				printf("Write beyond program size: %ld\n", par[j] - vm_memorysize + 1);
				printf("ip=%d base=%d tick=%d\n", ip, base, tick);
				#endif
				return ERR_SEGFAULT_WRITE;
			}
		}

		// Advance clock
		++tick;

		// Execute instruction
		switch (opcode)
		{
			case CPU_ADD: vm_memory[par[2]] = par[0] + par[1];  break;
			case CPU_MUL: vm_memory[par[2]] = par[0] * par[1];  break;
			case CPU_IN : vm_memory[par[0]] = vm_input();       break;
			case CPU_OUT: vm_output(par[0]);                    break;
			case CPU_JNZ: if ( par[0]) ip = par[1];             break;
			case CPU_JZ : if (!par[0]) ip = par[1];             break;
			case CPU_LT : vm_memory[par[2]] = par[0] < par[1];  break;
			case CPU_EQ : vm_memory[par[2]] = par[0] == par[1]; break;
			case CPU_RBO: base += par[0];                       break;
			case CPU_HLT: return ERR_OK;
			default :
				#ifdef DEBUG
				printf("Internal error: undefined opcode %d\n", opcode);
				#endif
				return ERR_OPCODE_UNDEFINED;
		}
	}
	return ERR_SEGFAULT_OTHER;
}

// Request value for input
long vm_input(void)
{
	static int count = 0;
	char *s = NULL;
	size_t t = 0;
	long val = -1;

	printf("%d ? ", count);
	if (getline(&s, &t, stdin) > 0)
		val = atol(s);
	free(s);
	count++;
	return val;
}

// Process value for output
void vm_output(long val)
{
	static int i = 0;
	char c = (char)val;

	printf("%c", c);
	if (i < MAZE_DIMX * MAZE_DIMY)
	{
		switch (c)
		{
			case '.': maze[i] = MAZE_WALL; break;
			case '#': maze[i] = MAZE_FREE; break;
			case '^':
			case 'v':
			case '<':
			case '>':
				orientation = maze_char2dir(c);
				maze[i] = MAZE_FREE;
				droidx = i % MAZE_DIMX;
				droidy = i / MAZE_DIMX;
				break;
			default: --i; break;
		}
		++i;
	}
}

char maze_dir2char(int dir)
{
	static const char *s = "X^v<>";

	if (dir >= MAZE_NODIR && dir <= MAZE_EAST)
		return s[dir];
	return '\0';
}

int maze_char2dir(char c)
{
	int dir = MAZE_NODIR;

	switch (c)
	{
		case '^': dir = MAZE_NORTH; break;
		case 'v': dir = MAZE_SOUTH; break;
		case '<': dir = MAZE_WEST;  break;
		case '>': dir = MAZE_EAST;  break;
	}
	return dir;
}

int maze_turnleft(int dir)
{
	int left = MAZE_NODIR;

	switch (dir)
	{
		case MAZE_NORTH: left = MAZE_WEST;  break;
		case MAZE_SOUTH: left = MAZE_EAST;  break;
		case MAZE_WEST : left = MAZE_SOUTH; break;
		case MAZE_EAST : left = MAZE_NORTH; break;
	}
	return left;
}

int maze_turnright(int dir)
{
	int right = MAZE_NODIR;

	switch (dir)
	{
		case MAZE_NORTH: right = MAZE_EAST;  break;
		case MAZE_SOUTH: right = MAZE_WEST;  break;
		case MAZE_WEST : right = MAZE_NORTH; break;
		case MAZE_EAST : right = MAZE_SOUTH; break;
	}
	return right;
}

int maze_newdir(int x, int y, int olddir)
{
	int dir = olddir;

	if (maze_peek(x, y, dir) == MAZE_WALL)
	{
		if maze_peek(x, y, (dir = maze_turnleft())
	}
	return dir;
}

int maze_index(int x, int y)
{
	return y * MAZE_DIMX + x;
}

int maze_peek(int x, int y, int dir)
{
	int i = -1;  // illegal index

	if (x < 0 || y < 0 || x >= MAZE_DIMX || y >= MAZE_DIMY)
		return MAZE_VOID;
	switch (dir)
	{
		case MAZE_NORTH: i = maze_index(x, y - 1); break;
		case MAZE_SOUTH: i = maze_index(x, y + 1); break;
		case MAZE_WEST : i = maze_index(x - 1, y); break;
		case MAZE_EAST : i = maze_index(x + 1, y); break;
	}
	if (i >= 0 && i < MAZE_DIMX * MAZE_DIMY)  // legal index?
		return maze[i];
	return MAZE_VOID;
}

int maze_alignment(int x, int y)
{
	int i;

	if (x <= 0 || y <= 0 || x >= MAZE_DIMX - 1 || y >= MAZE_DIMY - 1)
		return 0;  // can't be crossroads at outer border
	i = maze_index(x, y);
	if (maze[i] != MAZE_FREE) return 0;
	if (maze[i - 1] != MAZE_FREE) return 0;
	if (maze[i + 1] != MAZE_FREE) return 0;
	if (maze[i - MAZE_DIMX] != MAZE_FREE) return 0;
	if (maze[i + MAZE_DIMX] != MAZE_FREE) return 0;
	return x * y;
}

int maze_calibration(void)
{
	int x, y, sum = 0;

	for (y = 1; y < MAZE_DIMY - 1; ++y)
		for (x = 1; x < MAZE_DIMX - 1; ++x)
			sum += maze_alignment(x, y);
	return sum;
}

int maze_deadend(int x, int y)
{
	int dir, wallcount = 0;

	for (dir = MAZE_NORTH; dir <= MAZE_EAST; ++dir)
		if (maze_peek(x, y, dir) == MAZE_WALL)
			++wallcount;
	return wallcount >= 3;
}

// int maze_reverse(int dir)
// {
// 	int rev = MAZE_NODIR;

// 	switch (dir)
// 	{
// 		case MAZE_NORTH: rev = MAZE_SOUTH; break;
// 		case MAZE_SOUTH: rev = MAZE_NORTH; break;
// 		case MAZE_WEST : rev = MAZE_EAST;  break;
// 		case MAZE_EAST : rev = MAZE_WEST;  break;
// 	}
// 	return rev;
// }

// Draw maze
// void maze_draw(void)
// {
// 	int x, y;

// 	//printf("\033[2J");  // clear screen
// 	for (x = 0; x < MAZE_DIMX; ++x)
// 		printf("-");
// 	printf("--\n");

// 	for (y = 0; y < MAZE_DIMY; ++y)
// 	{
// 		printf("|");
// 		for (x = 0; x < MAZE_DIMX; ++x)
// 		{
// 			if (x == droidx && y == droidy)
// 				printf("o");
// 			else if (x == MAZE_X0 && y == MAZE_Y0)
// 				printf("+");
// 			else
// 				switch (maze[maze_index(x, y)])
// 				{
// 					case MAZE_VOID: printf(" "); break;
// 					case MAZE_WALL: printf("#"); break;
// 					case MAZE_FREE: printf("."); break;
// 					case MAZE_DEST: printf("X"); break;
// 				}
// 		}
// 		printf("|\n");
// 	}

// 	for (x = 0; x < MAZE_DIMX; ++x)
// 		printf("-");
// 	printf("--\n");
// }

////////// Main ///////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	int i, d, len, ret;

	// Init AoC
	srand(time(NULL));
	aoc_setfilename(aoc_thispuzzle(argv));

	// Init maze
	for (i = 0; i < MAZE_DIMX * MAZE_DIMY; ++i)
		maze[i] = MAZE_VOID;

	// Check program data
	if ((len = aoc_csvfields()) > 0)
	{
		// Allocate memory
		vm_cachesize  = len;
		vm_memorysize = len + VM_PAD;
		vm_cache  = malloc(vm_cachesize  * sizeof *vm_cache );
		vm_memory = malloc(vm_memorysize * sizeof *vm_memory);
		// Read into memory
		if (vm_cache != NULL && vm_memory != NULL && aoc_csv2cache() == len)
		{
			// Run program
			if ((ret = vm_exec(VM_RESET)) != ERR_OK)
				printf("Error: %d\n", ret);
			printf("droid: %d,%d %c\n", droidx, droidy, maze_dir2char(orientation));
			printf("calibration: %d\n", maze_calibration());
			printf("%d\n", maze_deadend(4, 28));
			printf("solution: ");
			do
			{
				if (maze_peek(droidx, droidy, (d = maze_dirleft(orientation))) == MAZE_FREE)
				{
					orientation = d;
					printf("L,");
				}
				else if (maze_peek(droidx, droidy, (d = maze_dirright(orientation))) == MAZE_FREE)
				{
					orientation = d;
					printf("R,");
				}
				i = 0;
				while (maze_peek(droidx, droidy, orientation) == MAZE_FREE)
				{
					switch (orientation)
					{
						case MAZE_NORTH: droidy--; break;
						case MAZE_SOUTH: droidy++; break;
						case MAZE_WEST : droidx--; break;
						case MAZE_EAST : droidx++; break;
					}
					++i;
				}
				printf("%d,", i);
			} while (i && !maze_deadend(droidx, droidy));
			printf("\n");
		}
		free(vm_memory);
		free(vm_cache);
	}
	return ERR_OK;
}
