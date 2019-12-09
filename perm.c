#include <stdio.h>

#define N 5
#define M 10

int main(void)
{
	int a, b, c, d, e, i = 0;

	for (a = N; a < M; ++a)
	{
		for (b = N; b < M; ++b)
		{
			if (b == a)
				continue;
			for (c = N; c < M; ++c)
			{
				if (c == a || c == b)
					continue;
				for (d = N; d < M; ++d)
				{
					if (d == a || d == b || d == c)
						continue;
					for (e = N; e < M; ++e)
					{
						if (e == a || e == b || e == c || e == d)
							continue;
						printf("%3d: %d %d %d %d %d\n", ++i, a, b, c, d, e);
					}
				}
			}
		}
	}
	return 0;
}
