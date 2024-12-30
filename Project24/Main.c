#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

// using FNV from https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned int FNV(const char* key)
{
	// See: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
	unsigned int h = 0x12345678L /* seed*/ ^ 2166136261UL;
	const char* data = (const char*)key;
	for (int i = 0; data[i] != '\0'; i++)
	{
		h ^= data[i];
		h *= 16777619;
	}
	return h;
}

#define NUM_HASHES_BUCKET (256)

struct HashBucket
{
	unsigned int hashes[NUM_HASHES_BUCKET];
	char pattern[NUM_HASHES_BUCKET][128];
	int value[NUM_HASHES_BUCKET];
};

#define NUM_HASHES_TABLE (1024)
struct HashTable
{
	struct HashBucket buckets[NUM_HASHES_TABLE];
};

void initHashTable(struct HashTable* hashTable)
{
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_TABLE; bucketIndex++)
	{
		memset(hashTable->buckets[bucketIndex].hashes, 0, sizeof(hashTable->buckets[bucketIndex].hashes));
		memset(hashTable->buckets[bucketIndex].pattern, 0, sizeof(hashTable->buckets[bucketIndex].pattern));
		memset(hashTable->buckets[bucketIndex].value, 0, sizeof(hashTable->buckets[bucketIndex].value));
	}
}

void addHashTable(struct HashTable* hashTable, const char* str, int value)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_BUCKET;

	int added = 0;
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashTable->buckets[bucket].pattern[bucketIndex][0] == 0 || // we need a valid non-zero pattern, so can use this to see if this is set
			(hashTable->buckets[bucket].hashes[bucketIndex] == hash && strcmp(hashTable->buckets[bucket].pattern[bucketIndex], str) == 0))
		{
			assert(strlen(str) < 127);
			hashTable->buckets[bucket].hashes[bucketIndex] = hash;
			strcpy(hashTable->buckets[bucket].pattern[bucketIndex], str);
			hashTable->buckets[bucket].value[bucketIndex] = value;
			added = 1;
			break;
		}
	}
	assert(added);
}

int* hasPatternHashTable(struct HashTable* hashTable, const char* str)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_BUCKET;

	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashTable->buckets[bucket].pattern[bucketIndex][0] == 0)
		{
			break; // this is empty, so it can't be here
		}
		if (hashTable->buckets[bucket].hashes[bucketIndex] == hash && strcmp(hashTable->buckets[bucket].pattern[bucketIndex], str) == 0) // strcmp might be unnecessary, but to be safe
		{
			return &hashTable->buckets[bucket].value[bucketIndex];
		}
	}

	return NULL;
}

struct Gate
{
	int computed;
	char name0[4];
	char name1[4];
	char operation[4];
	char output[4];
};

void initGate(struct Gate* gate, char* name0, char* name1, char* operation, char* output)
{
	gate->computed = 0;
	strcpy(gate->name0, name0);
	strcpy(gate->name1, name1);
	strcpy(gate->operation, operation);
	strcpy(gate->output, output);
}

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	struct HashTable* registerTable = malloc(sizeof(struct HashTable));
	initHashTable(registerTable);

#define MAX_GATES 256
	struct Gate gates[MAX_GATES];
	int numGates = 0;

	// we want to keep track of the number of final output registers so we can construct our number at the end
	int numFinalOutputRegisters = 0;

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int advIndex = 0;
		int nameIndex = 0;
		char name0[4] = { 0 };
		char name1[4] = { 0 };
		char output[4] = { 0 };
		char operation[4] = { 0 };

		if (input[0] == '\n') { continue; }

		while (isalnum(input[advIndex])) { name0[nameIndex] = input[advIndex]; nameIndex++; advIndex++; }

		// see if we are setting initial values or defining the graph
		if (input[advIndex] == ':')
		{
			while (!isalnum(input[advIndex])) { advIndex++; }

			// all initial values should be defining the register
			int value = atoi(&input[advIndex]);
			addHashTable(registerTable, name0, value);
		}
		else
		{
			while (!isalnum(input[advIndex])) { advIndex++; }

			nameIndex = 0;
			while (isalnum(input[advIndex])) { operation[nameIndex] = input[advIndex]; nameIndex++; advIndex++; }
			while (!isalnum(input[advIndex])) { advIndex++; }

			nameIndex = 0;
			while (isalnum(input[advIndex])) { name1[nameIndex] = input[advIndex]; nameIndex++; advIndex++; }
			while (!isalnum(input[advIndex])) { advIndex++; }

			nameIndex = 0;
			while (isalnum(input[advIndex])) { output[nameIndex] = input[advIndex]; nameIndex++; advIndex++; }

			initGate(&gates[numGates], name0, name1, operation, output);
			numGates++;

			// add any new registers
			if (!hasPatternHashTable(registerTable, name0))
			{
				addHashTable(registerTable, name0, -1);
			}
			if (!hasPatternHashTable(registerTable, name1))
			{
				addHashTable(registerTable, name1, -1);
			}
			if (!hasPatternHashTable(registerTable, output))
			{
				addHashTable(registerTable, output, -1);
			}

			// keep track of the number of final output registers
			if (output[0] == 'z')
			{
				numFinalOutputRegisters++;
			}
		}
	}

	// go through and compute all the gates to find all the register values
	// this will simply loop through everything and try to compute any gate that hasn't been computed yet
	// it's pretty inefficient, but there aren't enough gates to really care
	int waiting = 1;
	while (waiting)
	{
		waiting = 0;
		for (int gateIndex = 0; gateIndex < numGates; gateIndex++)
		{
			struct Gate* gate = &gates[gateIndex];
			if (!gate->computed)
			{
				int* valueIn0 = hasPatternHashTable(registerTable, gate->name0);
				int* valueIn1 = hasPatternHashTable(registerTable, gate->name1);
				int* valueOut = hasPatternHashTable(registerTable, gate->output);
				assert(valueIn0 != NULL && valueIn1 != NULL && valueOut != NULL);
				
				// see if we can compute this yet
				if (*valueIn0 != -1 && *valueIn1 != -1)
				{
					if (strcmp(gate->operation, "AND") == 0)
					{
						*valueOut = *valueIn0 && *valueIn1;
					}
					else if (strcmp(gate->operation, "OR") == 0)
					{
						*valueOut = *valueIn0 || *valueIn1;
					}
					else if (strcmp(gate->operation, "XOR") == 0)
					{
						*valueOut = *valueIn0 ^ *valueIn1;
					}
				}
				else
				{
					waiting = 1;
				}
			}
		}
	}

	// now find our secret number
	assert(numFinalOutputRegisters < 63);
	long long output = 0;
	for (int outputIndex = 0; outputIndex < numFinalOutputRegisters; outputIndex++)
	{
		char outputRegister[4];
		sprintf(outputRegister, "z%02d", outputIndex);
		int* outputValue = hasPatternHashTable(registerTable, outputRegister);
		assert(outputValue != NULL);
		output |= ((long long)(*outputValue)) << outputIndex; // set the specified bit to the value
	}
	printf("%lld\n", output);
}