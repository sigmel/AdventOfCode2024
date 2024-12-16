#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	long long total = 0;

#define INPUT_SIZE 256
	char input[INPUT_SIZE];

	do	{
		fgets(input, INPUT_SIZE, filePointer);
		int lineIndex = 12; // skip the Button part
		long long aX = atoll(&input[lineIndex]);
		while (isdigit(input[lineIndex])) { lineIndex++; }
		while (!isdigit(input[lineIndex])) { lineIndex++; }
		long long aY = atoll(&input[lineIndex]);

		fgets(input, INPUT_SIZE, filePointer);
		lineIndex = 12; // skip the Button part
		long long bX = atoll(&input[lineIndex]);
		while (isdigit(input[lineIndex])) { lineIndex++; }
		while (!isdigit(input[lineIndex])) { lineIndex++; }
		long long bY = atoll(&input[lineIndex]);

		fgets(input, INPUT_SIZE, filePointer);
		lineIndex = 9; // skip the Prize part
		long long pX = atoll(&input[lineIndex]);
		while (isdigit(input[lineIndex])) { lineIndex++; }
		while (!isdigit(input[lineIndex])) { lineIndex++; }
		long long pY = atoll(&input[lineIndex]);

		// add our unit conversion error
		pX += 10000000000000;
		pY += 10000000000000;

		// okay, fine, I'll do algebra
		// we can solve these for one variable and then compute the other
		// we have two equations here:
		// aX * numA + bX * numB = pX
		// aY * numA + bY * numB = pY
		// which on paper simplifies to:
		// (-aY/bY) * numA + (pY/bY) = (-aX/bX) * numA + (pX/bX) => ((-aY/bY) * numA + (pY/bY) = (-aX/bX) * numA + (pX/bX)) * (bX * bY)
		// (-aY*bX) * numA + (pY*bX) = (-aX*bY) * numA + (pX*bY) => (-aY*bX) * numA - (-aX*bY) * numA = (pX*bY) - (pY*bX)
		// numA = ((pX*bY) - (pY*bX)) / ((-aY*bX) - (-aX*bY))

		long long numANumerator = (pX * bY) - (pY * bX);
		long long numADemonerator = (-aY * bX) - (-aX * bY);

		long long numA = numANumerator / numADemonerator;

		// bX * numB = pX - (aX * numA) => numB = (pX - (aX * numA)) / bX
		long long numB = (pX - (aX * numA)) / bX;

		int foundResultX = (numA * aX + numB * bX == pX);
		int foundResultY = (numA * aY + numB * bY == pY);

		if (foundResultX && foundResultY)
		{
			total += numA * 3 + numB; // A presses cost 3 while B costs 1
		}
	} while (fgets(input, INPUT_SIZE, filePointer));

	printf("%lld\n", total);
}