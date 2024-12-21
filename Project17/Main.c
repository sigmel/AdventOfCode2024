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

void initVectorWithSize(struct Vector* vector, int initialSize)
{
	vector->count = 0;
	vector->size = initialSize;
	vector->data = malloc(sizeof(VECTOR_DATA) * vector->size);
}

void initVector(struct Vector* vector)
{
	const int VECTOR_INIT_SIZE = 10;
	initVectorWithSize(vector, VECTOR_INIT_SIZE);
}

void copyVector(struct Vector* dst, struct Vector* src)
{
	if (dst->size < src->count)
	{
		free(dst->data);
		dst->data = malloc(sizeof(VECTOR_DATA) * src->size);
		dst->size = src->size;
	}
	memcpy(dst->data, src->data, sizeof(VECTOR_DATA) * src->count);
	dst->count = src->count;
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

int containsValueInVector(struct Vector* vector, VECTOR_DATA value)
{
	int number = 0;
	for (int vectorIndex = 0; vectorIndex < vector->count; vectorIndex++)
	{
		if (vector->data[vectorIndex] == value)
		{
			number++;
		}
	}
	return number;
}

void clearVector(struct Vector* vector)
{
	vector->count = 0;
	//memset(vector->data, 0, sizeof(VECTOR_DATA) * vector->size);
}

#define OPCODE_ADV 0
#define OPCODE_BXL 1
#define OPCODE_BST 2
#define OPCODE_JNZ 3
#define OPCODE_BXC 4
#define OPCODE_OUT 5
#define OPCODE_BDV 6
#define OPCODE_CDV 7

long long comboOperand(int operand, long long registerA, long long registerB, long long registerC)
{
	if (operand >= 0 && operand <= 3) { return operand; }
	if (operand == 4) { return registerA; }
	if (operand == 5) { return registerB; }
	if (operand == 6) { return registerC; }
	assert(0); // operand 7 is invalid, unsure if I need to do anything with it yet so this will catch it
	return -1;
}

void main()
{
	struct Vector programVector;
	initVector(&programVector);

	FILE* filePointer = fopen("puzzle.txt", "r");

#define INPUT_SIZE 256
	char input[INPUT_SIZE];
	int advIndex = 0;

	// register A
	fgets(input, INPUT_SIZE, filePointer);
	while (!isdigit(input[advIndex])) { advIndex++; }
	long long registerA = atoi(&input[advIndex]);

	// register B
	advIndex = 0;
	fgets(input, INPUT_SIZE, filePointer);
	while (!isdigit(input[advIndex])) { advIndex++; }
	long long registerB = atoi(&input[advIndex]);

	// register C
	advIndex = 0;
	fgets(input, INPUT_SIZE, filePointer);
	while (!isdigit(input[advIndex])) { advIndex++; }
	long long registerC = atoi(&input[advIndex]);

	// blank line
	fgets(input, INPUT_SIZE, filePointer);

	// program
	advIndex = 0;
	fgets(input, INPUT_SIZE, filePointer);
	while (input[advIndex] != 0)
	{
		while (!isdigit(input[advIndex])) { advIndex++; }
		int opcode = atoi(&input[advIndex]);
		while (isdigit(input[advIndex])) { advIndex++; }
		addVector(&programVector, opcode);
	}
	fclose(filePointer);

	struct Vector outVector;
	initVector(&outVector);

	// Every time we loop on this, we divide A by 8, so we can sort of do that in reverse, we we find the digit we want
	// (starting from the back), then multiply by 8, proceed to find the next digit, and so on until we have all of them
	int instructionPointer = 0;
	int numCompareDigits = 1;
	long long initialA = 0;
	int done = 0;
	while (!done)
	{
		registerA = initialA;
		registerB = 0;
		registerC = 0;
		while (instructionPointer < programVector.count)
		{
			int opcode = programVector.data[instructionPointer];
			switch (opcode)
			{
				case OPCODE_ADV:
				{
					// The adv instruction(opcode 0) performs division. The numerator is the value in the A register.
					// The denominator is found by raising 2 to the power of the instruction's combo operand.
					// (So, an operand of 2 would divide A by 4 (2^2); an operand of 5 would divide A by 2^B.)
					// The result of the division operation is truncated to an integer and then written to the A register.
					if (instructionPointer + 1 < programVector.count)
					{
						long long denominator = 1LL << comboOperand(programVector.data[instructionPointer + 1], registerA, registerB, registerC);
						registerA /= denominator;
					}

					instructionPointer += 2;
					break;
				}
				case OPCODE_BXL:
				{
					// The bxl instruction (opcode 1) calculates the bitwise XOR of register B and the instruction's literal operand,
					// then stores the result in register B.
					if (instructionPointer + 1 < programVector.count)
					{
						registerB ^= programVector.data[instructionPointer + 1];
					}

					instructionPointer += 2;
					break;
				}
				case OPCODE_BST:
				{
					// The bst instruction (opcode 2) calculates the value of its combo operand modulo 8 (thereby keeping only its lowest 3 bits),
					// then writes that value to the B register.
					if (instructionPointer + 1 < programVector.count)
					{
						registerB = comboOperand(programVector.data[instructionPointer + 1], registerA, registerB, registerC) % 8;
					}

					instructionPointer += 2;
					break;
				}
				case OPCODE_JNZ:
				{
					// The jnz instruction (opcode 3) does nothing if the A register is 0.
					// However, if the A register is not zero, it jumps by setting the instruction pointer to the value of its literal operand;
					// if this instruction jumps, the instruction pointer is not increased by 2 after this instruction.
					if (registerA != 0 && instructionPointer + 1 < programVector.count)
					{
						instructionPointer = programVector.data[instructionPointer + 1];
					}
					else
					{
						instructionPointer += 2;
					}
					break;
				}
				case OPCODE_BXC:
				{
					// The bxc instruction (opcode 4) calculates the bitwise XOR of register B and register C, then stores the result in register B.
					// (For legacy reasons, this instruction reads an operand but ignores it.)
					if (instructionPointer + 1 < programVector.count)
					{
						registerB = registerB ^ registerC;
					}

					instructionPointer += 2;
					break;
				}
				case OPCODE_OUT:
				{
					// The out instruction (opcode 5) calculates the value of its combo operand modulo 8, then outputs that value.
					// (If a program outputs multiple values, they are separated by commas.)
					if (instructionPointer + 1 < programVector.count)
					{
						addVector(&outVector, comboOperand(programVector.data[instructionPointer + 1], registerA, registerB, registerC) % 8);
					}

					instructionPointer += 2;
					break;
				}
				case OPCODE_BDV:
				{
					// The bdv instruction (opcode 6) works exactly like the adv instruction except that the result is stored in the B register.
					// (The numerator is still read from the A register.)
					if (instructionPointer + 1 < programVector.count)
					{
						long long denominator = 1LL << comboOperand(programVector.data[instructionPointer + 1], registerA, registerB, registerC);
						registerB = registerA / denominator;
					}

					instructionPointer += 2;
					break;
				}
				case OPCODE_CDV:
				{
					// The cdv instruction (opcode 7) works exactly like the adv instruction except that the result is stored in the C register.
					// (The numerator is still read from the A register.)
					if (instructionPointer + 1 < programVector.count)
					{
						long long denominator = 1LL << comboOperand(programVector.data[instructionPointer + 1], registerA, registerB, registerC);
						registerC = registerA / denominator;
					}

					instructionPointer += 2;
					break;
				}
			}
		}

		/*
		printf("%lld - ", initialA);
		for (int outIndex = 0; outIndex < outVector.count - 1; outIndex++)
		{
			printf("%d, ", outVector.data[outIndex]);
		}
		printf("%d\n", outVector.data[outVector.count - 1]);
		*/
		if (memcmp(outVector.data, &programVector.data[programVector.count - numCompareDigits], sizeof(int) * numCompareDigits) == 0)
		{
			if (programVector.count == outVector.count)
			{
				break;
			}
			initialA *= 8;
			numCompareDigits++;
		}
		else
		{
			initialA++;
		}

		instructionPointer = 0;
		clearVector(&outVector);
	}

	assert(programVector.count == outVector.count && memcmp(programVector.data, outVector.data, sizeof(int)* programVector.count) == 0);
	printf("%lld\n", initialA);
}