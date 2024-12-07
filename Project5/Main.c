#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define VECTOR_DATA int
struct Vector
{
	int count;
	int size;
	VECTOR_DATA* data;
};

void initVector(struct Vector* vector)
{
	const int VECTOR_INIT_SIZE = 10;
	vector->count = 0;
	vector->size = VECTOR_INIT_SIZE;
	vector->data = malloc(sizeof(VECTOR_DATA) * vector->size);
}

void copyVector(struct Vector* dst, struct Vector* src)
{
	dst->data = malloc(sizeof(VECTOR_DATA) * src->size);
	if (dst->data)
	{
		memcpy(dst->data, src->data, sizeof(VECTOR_DATA) * src->size);
		dst->count = src->count;
		dst->size = src->size;
	}
}

void freeVector(struct Vector* vector)
{
	vector->count = 0;
	vector->size = 0;
	free(vector->data);
}

void addVector(struct Vector* vector, VECTOR_DATA value)
{
	if (vector->count == vector->size)
	{
		vector->size *= 2;
		VECTOR_DATA* newData = malloc(sizeof(VECTOR_DATA) * vector->size);
		if (newData)
		{
			memcpy(newData, vector->data, sizeof(VECTOR_DATA) * vector->count);
			free(vector->data);
			vector->data = newData;
		}
	}

	vector->data[vector->count] = value;
	vector->count++;
}

void removeElementAtVector(struct Vector* vector, int index)
{
	assert(index < vector->count);
	for (int vectorIndex = index; vectorIndex < vector->count - 1; vectorIndex++)
	{
		vector->data[vectorIndex] = vector->data[vectorIndex + 1];
	}
	vector->count--;
}

void clearVector(struct Vector* vector)
{
	vector->count = 0;
	memset(vector->data, 0, sizeof(VECTOR_DATA) * vector->size);
}

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	struct Vector rules;
	initVector(&rules);

	// read the page ordering rules
	#define INPUT_SIZE 256
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		// check if we've reached the end of the section
		if (input[0] == '\n')
		{
			break;
		}

		int advIndex = 0;
		int value = atoi(&input[advIndex]);
			
		addVector(&rules, value);

		while (isdigit(input[advIndex])) { advIndex++; }
		advIndex++; // skip past the '|' character

		value = atoi(&input[advIndex]);
		addVector(&rules, value);
	}

	// now verify all the page ordering
	int sum = 0;
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		struct Vector inputVector;
		initVector(&inputVector);

		// get the pages
		int value = 0;
		int advIndex = 0;
		do
		{
			value = atoi(&input[advIndex]);
			addVector(&inputVector, value);

			while (isdigit(input[advIndex])) { advIndex++; }
			while (input[advIndex] == ',' || input[advIndex] == '\n') { advIndex++; }
		} while (input[advIndex] != 0);

		// now validate them (if I cared about speed, then I could have made a map of vectors or something in C++)
		int valid = 1;

	startLoop: // kickin' it super old school
		for (int numberIndex = 0; numberIndex < inputVector.count; numberIndex++)
		{
			int number = inputVector.data[numberIndex];
			for (int ruleIndex = 0; ruleIndex < rules.count; ruleIndex += 2)
			{
				if (number == rules.data[ruleIndex])
				{
					// make sure everything is in order (only need to check if the pages before violate the rule)
					int afterNumber = rules.data[ruleIndex + 1];
					for (int checkIndex = 0; checkIndex < numberIndex; checkIndex++)
					{
						if (inputVector.data[checkIndex] == afterNumber)
						{
							// just going to swap these and start the checks again cause computers these days are fast
							inputVector.data[checkIndex] = inputVector.data[numberIndex];
							inputVector.data[numberIndex] = afterNumber;
							valid = 0;
							goto startLoop;
						}
					}
				}
			}
		}

		if (!valid) // only sum invalid ones now
		{
			sum += inputVector.data[inputVector.count / 2];
		}

		freeVector(&inputVector);
	}

	printf("%d\n", sum);
}