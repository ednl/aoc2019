#include <stdio.h>

#define W 25
#define H 6
#define P (W * H)

int main(void)
{
	FILE *fp;
	char c;
	int i, k, bit[P];

	if ((fp = fopen("inp08.txt", "r")) != NULL)
	{
		for (i = 0; i < P; ++i)
			bit[i] = 2;

		i = 0;
		while ((c = fgetc(fp)) != EOF)
			if ((k = c - '0') >= 0 && k < 3)
			{
				if (k != 2 && bit[i] == 2)
					bit[i] = k;
				if (++i == P)
					i = 0;
			}

		i = 0;
		while (i < P)
		{
			printf("%c", bit[i++] ? '*' : ' ');
			if (!(i % W))
				printf("\n");
		}
	}

	return 0;
}
