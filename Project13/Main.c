#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	int total = 0;

#define INPUT_SIZE 256
	char input[INPUT_SIZE];

	do	{
		fgets(input, INPUT_SIZE, filePointer);
		int lineIndex = 12; // skip the Button part
		int aX = atoi(&input[lineIndex]);
		while (isdigit(input[lineIndex])) { lineIndex++; }
		while (!isdigit(input[lineIndex])) { lineIndex++; }
		int aY = atoi(&input[lineIndex]);

		fgets(input, INPUT_SIZE, filePointer);
		lineIndex = 12; // skip the Button part
		int bX = atoi(&input[lineIndex]);
		while (isdigit(input[lineIndex])) { lineIndex++; }
		while (!isdigit(input[lineIndex])) { lineIndex++; }
		int bY = atoi(&input[lineIndex]);

		fgets(input, INPUT_SIZE, filePointer);
		lineIndex = 9; // skip the Prize part
		int pX = atoi(&input[lineIndex]);
		while (isdigit(input[lineIndex])) { lineIndex++; }
		while (!isdigit(input[lineIndex])) { lineIndex++; }
		int pY = atoi(&input[lineIndex]);

		// since B costs less than A, we'll see what the max for that would be, then add in A's until we reach it or bail
		// since these are linear, we can just look at one dimension for comparison (and just update the other)
		int numA = 0;
		int numB = pX / bX;

		int resultX = numB * bX;
		int resultY = numB * bY;
		int foundResult = 1; // assume true until proven otherwise
		while (resultX != pX || resultY != pY)
		{
			if (numB == 0)
			{
				// couldn't find a result, so need to bail
				foundResult = 0;
				break;
			}

			if (resultX > pX)
			{
				numB--;
			}
			else
			{
				numA++;
			}
			resultX = numA * aX + numB * bX;
			resultY = numA * aY + numB * bY;
		}

		if (foundResult)
		{
			total += numA * 3 + numB; // A presses cost 3 while B costs 1
		}
	} while (fgets(input, INPUT_SIZE, filePointer));

	printf("%d\n", total);
}