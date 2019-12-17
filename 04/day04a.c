#include <stdio.h>

#define START 197487u
#define STOP  673251u
#define LEN 6

int main(void)
{
	unsigned int n, m, i, k, p = 0;
	unsigned int d[LEN];

	for (n = START; n <= STOP; ++n) {
		k = 0;
		d[0] = n % 10;
		m = n / 10;
		for (i = 1; i < LEN; ++i) {
			d[i] = m % 10;
			if (d[i] > d[i - 1])
				goto nextn;
			else if (d[i] == d[i - 1])
				++k;
			m /= 10;
		}
		if (k)
			++p;
nextn:;
	}
	printf("%u\n", p);
	return 0;
}
