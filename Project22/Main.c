#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

long long mix(long long secret, long long value)
{
	// To mix a value into the secret number, calculate the bitwise XOR of the given value and the secret number.
	// Then, the secret number becomes the result of that operation.
	// (If the secret number is 42 and you were to mix 15 into the secret number, the secret number would become 37.)
	return secret ^ value;
}

long long prune(long long number)
{
	// To prune the secret number, calculate the value of the secret number modulo 16777216.
	// Then, the secret number becomes the result of that operation.
	// (If the secret number is 100000000 and you were to prune the secret number, the secret number would become 16113920.)
	return number % 16777216;
}

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	long long total = 0;

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		long long secret = atoll(input);

		const int iterationNum = 2000;

		for (int iterateIndex = 0; iterateIndex < iterationNum; iterateIndex++)
		{
			// Calculate the result of multiplying the secret number by 64. Then, mix this result into the secret number. Finally, prune the secret number.
			long long result = secret * 64;
			secret = mix(secret, result);
			secret = prune(secret);

			// Calculate the result of dividing the secret number by 32. Round the result down to the nearest integer.Then, mix this result into the secret number.Finally, prune the secret number.
			result = secret / 32;
			secret = mix(secret, result);
			secret = prune(secret);

			// Calculate the result of multiplying the secret number by 2048. Then, mix this result into the secret number.Finally, prune the secret number.
			result = secret * 2048;
			secret = mix(secret, result);
			secret = prune(secret);
		}

		total += secret;
	}
	fclose(filePointer);

	printf("%lld\n", total);
}