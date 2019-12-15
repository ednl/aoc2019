///////////////////////////////////////////////////////////////////////////////
// Advent of Code 2019
// Day 13: Care Package
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
#include <time.h>    // time for srand

#define DEBUG
#define PAD 1000  // storage space to add at end of program
#define RESUME 0
#define RESET  1

#define ERR_OK               0
#define ERR_OPCODE_UNKNOWN   1
#define ERR_OPCODE_UNDEFINED 2
#define ERR_SEGFAULT_PARAM   3
#define ERR_SEGFAULT_READ    4
#define ERR_SEGFAULT_WRITE   5
#define ERR_SEGFAULT_OTHER   6

// Opcodes
#define ADD  1L  // add
#define MUL  2L  // multiply
#define IN   3L  // get input
#define OUT  4L  // send output
#define JNZ  5L  // jump non-zero
#define JZ   6L  // jump zero
#define LT   7L  // compare less than
#define EQ   8L  // compare equal
#define RBO  9L  // relative base offset
#define HLT 99L  // return (halt)

// Parameters
#define POS  0L  // positional
#define IMM  1L  // immediate
#define OFF  2L  // offset
#define ARG  3L  // max number of parameters per opcode

// FIFO operation
#define FIFOLEN  5000  // length of the FIFO buffer
#define INOUT_BOTH  0  // first try FIFO buffer, then terminal
#define INOUT_FIFO  1  // only try FIFO buffer input/output
#define INOUT_TERM  2  // only do terminal input/output
#define FLUSH_HIDE  0  // don't print to term when flushing
#define FLUSH_SHOW  1  // print flushed values to terminal

// Game
#define NORTH  1
#define SOUTH  2
#define WEST   3
#define EAST   4
#define TODO  -1
#define WALL   0
#define EMPTY  1
#define TARGET 2
#define XDIM  80
#define YDIM  40

////////// Typedefs & Constants ///////////////////////////////////////////////

// Location of program on disk
static const char *inp = "inp15.txt";

typedef struct Coor {
	int x, y;
} COOR, *PCOOR;

// Input/output values + counter
typedef struct Dict {
	int key;
	long val;
} DICT, *PDICT;

// Language entries
typedef struct Lang {
	long op;
	int rp, wp;
} LANG;

// Language definition: { opcode, read count, write count }
// Every read parameter can be positional or immediate
// Write parameters are always positional
static const LANG lang[] = {
	{ ADD, 2, 1 },
	{ MUL, 2, 1 },
	{ IN , 0, 1 },
	{ OUT, 1, 0 },
	{ JNZ, 2, 0 },
	{ JZ , 2, 0 },
	{ LT , 2, 1 },
	{ EQ , 2, 1 },
	{ RBO, 1, 0 },
	{ HLT, 0, 0 }
};
static const int langsize = sizeof lang / sizeof *lang;

////////// Globals ////////////////////////////////////////////////////////////

static DICT fifobuf[FIFOLEN];
static int fifohead = 0, fifotail = 0, fifosize = 0;

static long *dat = NULL, *mem = NULL;
static int datsize = 0, memsize = 0;

static int xbot = XDIM / 2, ybot = YDIM / 2;
static int xtarget = -1, ytarget = -1;
static int move = NORTH, steps = 0;
static int maze[XDIM * YDIM];

////////// Function Declarations //////////////////////////////////////////////

int fifo_pop(PDICT);
int fifo_push(DICT);
int fifo_flush(int);
void output(long, int);
long input(int);
int sizecsv(void);
int readcsv(void);
void copyprog(void);
int execprog(int);
void drawmaze(void);
int getindex(int, int);
COOR getpos(int);
int reverse(int);
int peek(int);
void makemove(int);

////////// Function Definitions ///////////////////////////////////////////////

// Retrieve value from FIFO buffer
// Ret: 1=success, 0=buffer empty
int fifo_pop(PDICT a)
{
	if (!fifosize)
	{
		#ifdef DEBUG
		printf("Can't pop from empty buffer\n");
		#endif
		return 0;
	}
	*a = fifobuf[fifotail];
	fifotail = (fifotail + 1) % FIFOLEN;
	--fifosize;
	return 1;
}

// Save value to FIFO buffer
// Ret: 1=success, 0=buffer full
int fifo_push(DICT a)
{
	if (fifosize == FIFOLEN)
	{
		#ifdef DEBUG
		printf("Can't push to full buffer\n");
		#endif
		return 0;
	}
	fifobuf[fifohead] = a;
	fifohead = (fifohead + 1) % FIFOLEN;
	++fifosize;
	return 1;
}

// Empty the FIFO buffer
// Arg: 1=print key/val to terminal, 0=don't print
// Ret: number of values cleared
int fifo_flush(int print)
{
	DICT a;
	int old;

	if ((old = fifosize))
	{
		if (print)
		{
			while (fifosize)
				if (fifo_pop(&a))
					output(a.val, INOUT_TERM);
		} else
			fifotail = fifohead;
		fifosize = 0;
	}
	return old;
}

// Process value for output
// Arg: a = counter + output value
//      mode 0 = try push to FIFO, then print
//           1 = only push to FIFO buffer
//           2 = only print to terminal
void output(long val, int mode)
{
	static int count = 0;
	int ok = 1;

	if (mode == 0 || mode == 1)
		ok = fifo_push((DICT){ count, val });
	if (mode == 2 || (mode == 0 && !ok))
		printf("%d : %ld\n", count, val);
	++count;
}

// Request value for input
// Arg: mode 0 = try pop from FIFO, then ask
//           1 = only pop from FIFO buffer
//           2 = only ask on terminal
// Ret: counter + input value
long input(int mode)
{
	static int count = 0;
	char *s = NULL;
	size_t t = 0;
	DICT a;
	long val = -1;
	int ok = 1;

	if (mode == 0 || mode == 1)
		if ((ok = fifo_pop(&a)))
			val = a.val;
	if (mode == 2 || (mode == 0 && !ok))
	{
		printf("%d ? ", count);
		if (getline(&s, &t, stdin) > 0)
			val = atol(s);
		free(s);
	}
	++count;
	return val;
}

// Count values in a one-line CSV file by counting commas + 1
int sizecsv(void)
{
	FILE *fp;
	int ch, i = 0;

	if ((fp = fopen(inp, "r")) != NULL)
	{
		i = 1;
		while ((ch = fgetc(fp)) != EOF)
			if (ch == ',')
				++i;
		fclose(fp);
	}
	return i;
}

// Read long integer CSV values from file to array dat
// Pre: dat must be allocated to size datsize > 0
//      inp must contain name of readable file
// Ret: number of values read
int readcsv(void)
{
	FILE *fp;        // file pointer
	char *s = NULL;  // dynamically allocated buffer
	size_t t = 0;    // size of buffer
	int i = 0;       // values read

	if (dat != NULL && datsize > 0 && (fp = fopen(inp, "r")) != NULL)
	{
		while (i < datsize && getdelim(&s, &t, ',', fp) > 0)
			dat[i++] = atol(s);
		free(s);
		fclose(fp);
	}
	return i;
}

// Copy program from dat to mem and zero the padding
// Pre: dat must be allocated to size datsize > 0
//      mem must be allocated to size memsize >= datsize
void copyprog(void)
{
	long *src, *dst;
	int i;

	if (dat != NULL && mem != NULL && datsize > 0 && datsize <= memsize)
	{
		src = dat;
		dst = mem;
		i = datsize;
		while (i--)
			*dst++ = *src++;
		i = memsize - datsize;
		while (i--)
			*dst++ = 0;
	}
}

// Parse and execute all instructions in memory
int execprog(int reset)
{
	static long base = 0;
	static int pc = 0;

	int i, j, k, retval = ERR_OK;
	long in, op, par[ARG], parmode;

	if (reset)
	{
		copyprog();
		base = 0;
		pc = 0;
	}

	while (pc >= 0 && pc < memsize)
	{
		// Get opcode and parameter modes from instruction
		in = mem[pc++];                // get instruction, increment program counter
		op = in % 100;                 // opcode part of instruction
		in /= 100;                     // this leaves parameter modes

		// Look up opcode, halt if not found or too long
		i = 0;
		while (i < langsize && op != lang[i].op)
			++i;
		if (i == langsize)             // not in the language def?
		{
			#ifdef DEBUG
			printf("Unknown opcode %ld at %d\n", op, pc - 1);
			#endif
			return ERR_OPCODE_UNKNOWN;
		}
		if (pc + lang[i].rp + lang[i].wp > memsize)
		{
			#ifdef DEBUG
			printf("Too many parameters for program size at %d\n", pc - 1);
			#endif
			return ERR_SEGFAULT_PARAM;
		}

		// Get read parameter(s)
		for (j = 0; j < lang[i].rp; ++j)
		{
			par[j] = mem[pc++];        // get immediate value param, increment program counter
			parmode = in % 10;
			in /= 10;                  // next parameter mode
			if (parmode != IMM)        // mode = positional or offset parameter?
			{
				if (parmode == OFF)
					par[j] += base;
				if (par[j] < 0 || par[j] >= memsize)
				{
					#ifdef DEBUG
					printf("Read beyond program size: %ld\n", par[j] - memsize + 1);
					#endif
					return ERR_SEGFAULT_READ;
				}
				par[j] = mem[par[j]];  // get positional value
			}
		}

		// Get write parameter (always positional/offset but keep the address this time)
		if (lang[i].wp)
		{
			par[j] = mem[pc++];        // get address param, increment program counter
			if (in % 10 == OFF)        // mode = offset parameter?
				par[j] += base;
			if (par[j] < 0 || par[j] >= memsize)
			{
				#ifdef DEBUG
				printf("Write beyond program size: %ld\n", par[j] - memsize + 1);
				#endif
				return ERR_SEGFAULT_WRITE;
			}
		}

		// Execute
		switch (op)
		{
			case ADD: mem[par[2]] = par[0] + par[1];  break;
			case MUL: mem[par[2]] = par[0] * par[1];  break;
			case IN : mem[par[0]] = move;             break;
			case OUT: makemove((int)par[0]);          break;
			case JNZ: if ( par[0]) pc = par[1];       break;
			case JZ : if (!par[0]) pc = par[1];       break;
			case LT : mem[par[2]] = par[0] < par[1];  break;
			case EQ : mem[par[2]] = par[0] == par[1]; break;
			case RBO: base += par[0];                 break;
			case HLT: return retval;
			default :
				#ifdef DEBUG
				printf("Internal error: undefined opcode %ld\n", op);
				#endif
				return ERR_OPCODE_UNDEFINED;
		}
	}
	return ERR_SEGFAULT_OTHER;
}

// Draw maze
void drawmaze(void)
{
	static int xmin = XDIM - 1, xmax = 0;
	static int ymin = YDIM - 1, ymax = 0;
	int x, y, i;

	printf("\033[2J");  // clear screen
	i = 0;
	for (y = 0; y < YDIM; ++y)
		for (x = 0; x < XDIM; ++x)
		{
			if (maze[i] != TODO)
			{
				if (x < xmin) xmin = x;
				if (x > xmax) xmax = x;
				if (y < ymin) ymin = y;
				if (y > ymax) ymax = y;
			}
			++i;
		}

	i = 0;
	for (y = ymin; y <= ymax; ++y)
	{
		for (x = xmin; x <= xmax; ++x)
		{
			if (x == xbot && y == ybot)
				printf("D");
			else
				switch (maze[i])
				{
					case TODO  : printf(" "); break;
					case WALL  : printf("#"); break;
					case EMPTY : printf("."); break;
					case TARGET: printf("X"); break;
				}
			++i;
		}
		printf("\n");
	}
}

int getindex(int x, int y)
{
	return y * XDIM + x;
}

COOR getpos(int ix)
{
	return (COOR){ ix % XDIM, ix / XDIM };
}

int reverse(int dir)
{
	int r = 0;

	switch (dir)
	{
		case NORTH: r = SOUTH; break;
		case SOUTH: r = NORTH; break;
		case WEST : r = EAST;  break;
		case EAST : r = WEST;  break;
	}
	return r;
}

int peek(int dir)
{
	int i = -1;

	switch (dir)
	{
		case NORTH: i = getindex(xbot, ybot - 1); break;
		case SOUTH: i = getindex(xbot, ybot + 1); break;
		case WEST : i = getindex(xbot - 1, ybot); break;
		case EAST : i = getindex(xbot + 1, ybot); break;
	}
	if (i >= 0 && i < XDIM * YDIM)
		return maze[i];
	else
		return WALL;
}

void makemove(int resp)
{
	int i, m, r, skip, x = xbot, y = ybot;
	int todo[4], todocount = 0;
	int been[4], beencount = 0;

	switch (move)
	{
		case NORTH: y--; break;
		case SOUTH: y++; break;
		case WEST : x--; break;
		case EAST : x++; break;
	}

	i = getindex(x, y);
	if (i < 0 || i >= XDIM * YDIM)
	{
		#ifdef DEBUG
		drawmaze();
		printf("Illegal move to %d,%d (resp=%d)\n", x, y, resp);
		#endif
		exit(1);
	}

	maze[i] = resp;
	if (resp != WALL)
	{
		xbot = x;
		ybot = y;
	}
	if (resp == TARGET)
	{
		drawmaze();
		printf("Found target at %d,%d\n", x, y);
		exit(0);
	}
	for (m = NORTH; m <= EAST; ++m)
	{
		switch (peek(m))
		{
			case TODO  : todo[todocount++] = m; break;
			case EMPTY :
			case TARGET: been[beencount++] = m; break;
		}
	}
	if (todocount == 1)
		m = todo[0];
	else if (todocount > 1)
		m = todo[rand() / ((RAND_MAX + 1U) / todocount)];
	else if (beencount == 1)
		m = been[0];
	else if (beencount > 1)
	{
		r = reverse(move);
		skip = 4;
		for (i = 0; i < beencount; ++i)
			if (been[i] == r)
			{
				skip = i;
				beencount--;
				break;
			}
		i = beencount == 1 ? 0 : rand() / ((RAND_MAX + 1U) / beencount);
		if (i >= skip)
			++i;
		m = been[i];
	} else
	{
		#ifdef DEBUG
		drawmaze();
		printf("No legal moves from %d,%d\n", xbot, ybot);
		#endif
		exit(1);
	}
	move = m;
}

////////// Main ///////////////////////////////////////////////////////////////

int main(void)
{
	int i, len, ret;

	srand(time(NULL));
	for (i = 0; i < XDIM * YDIM; ++i)
		maze[i] = TODO;

	if ((len = sizecsv()) > 0)
	{
		datsize = len;
		memsize = len + PAD;
		dat = malloc(datsize * sizeof *dat);
		mem = malloc(memsize * sizeof *mem);
		if (dat != NULL && mem != NULL && readcsv() == len)
		{
			ret = execprog(RESET);
			printf("ret = %d\n", ret);
		}
		free(mem);
		free(dat);
	}
	return 0;
}
