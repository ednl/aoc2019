#include <stdio.h>

#define START 197487u
#define STOP  673251u

int main(void)
{
	unsigned int n, m, i, p, q, r, s, z = 0;

	for (n = START; n <= STOP; ++n) {
	// for (n = 223333; n <= 223333; ++n) {
		p = n % 10;               // current digit
		m = n / 10;               // shift right
		r = 1;                    // current repeat count
		s = 0;                    // number of 2-peats
		while (m) {
			q = m % 10;           // next digit
			// printf("%u %u %u %u %u\n", n, p, q, r, s);
			if (q > p) {          // not a valid pwd
				goto nextn;
			} else if (q == p) {  // repeated digit?
				++r;
			} else {              // different digit
				if (r == 2)       // old repeat count is good?
					++s;          // another 2-peat
				p = q;            // current digit
				r = 1;            // current repeat count
			}
			m /= 10;              // shift right
		}
		if (s || r == 2) {
			// printf("%u\n", n);
			++z;
		}
nextn:;
	}
	printf("%u\n", z);
	return 0;
}
