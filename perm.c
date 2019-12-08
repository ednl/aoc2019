#include <stdio.h>

#define N 5

void print(int *a)
{
	int i;

	for (i = 0; i < N; ++i)
		printf("%d", a[i]);
	printf("\n");
}

void swap(int *a, int i, int j)
{
	if (i >= 0 && j >= 0 && i < N && j < N)
	{
		int t = a[j];
		a[j] = a[i];
		a[i] = t;
	}
}

int main(void)
{
	int p[N] = {0, 1, 2, 3, 4};
	int q[N] = {0, 0, 0, 0, 0};
	int i = 0, j = 0;

	printf("%3d: ", ++j);
	print(p);

	while (i < N)
	{
		if (q[i] < i)
		{
			if (i % 2)
				swap(p, q[i], i);
			else
				swap(p, 0, i);

			printf("%3d: ", ++j);
			print(p);

			q[i] += 1;
			i = 0;
		}
		else
		{
			q[i] = 0;
			i += 1;
		}
	}
	return 0;
}
