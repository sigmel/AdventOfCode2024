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

struct Gate* findGateByOutput(struct Gate* gates, int numGates, const char* output)
{
	for (int gateIndex = 0; gateIndex < numGates; gateIndex++)
	{
		if (strcmp(gates[gateIndex].output, output) == 0)
		{
			return &gates[gateIndex];
		}
	}

	return NULL;
}

void findGatesByInput(struct Gate* gates, int numGates, const char* input, struct Gate** inputGates, int* numInputGates, int maxInputGates)
{
	for (int gateIndex = 0; gateIndex < numGates; gateIndex++)
	{
		if (strcmp(gates[gateIndex].name0, input) == 0 ||
			strcmp(gates[gateIndex].name1, input) == 0)
		{
			assert(*numInputGates < maxInputGates);
			inputGates[*numInputGates] = &gates[gateIndex];
			(*numInputGates)++;
		}
	}
}

void buildGateGraphByOutput(struct Gate* gates, int numGates, const char* output, struct Gate* graphGates, int* numGraphGates)
{
	struct Gate* findGate = findGateByOutput(gates, numGates, output);
	assert(findGate != NULL);

	// make sure we haven't added this yet
	if (findGateByOutput(graphGates, *numGraphGates, output) == NULL)
	{
		initGate(&graphGates[*numGraphGates], findGate->name0, findGate->name1, findGate->operation, findGate->output);
		(*numGraphGates)++;
	}

	// if we have two inputs, then this is the end of the graph in this direction
	if (findGate->name0[0] != 'x' && findGate->name0[0] != 'y')
	{
		buildGateGraphByOutput(gates, numGates, findGate->name0, graphGates, numGraphGates);
	}
	if (findGate->name1[0] != 'x' && findGate->name1[0] != 'y')
	{
		buildGateGraphByOutput(gates, numGates, findGate->name1, graphGates, numGraphGates);
	}
}

int computeGates(struct Gate* gates, int numGates, struct HashTable* registerTable)
{
	// go through and compute all the initialGates to find all the register values
	// this will simply loop through everything and try to compute any gate that hasn't been computed yet
	// it's pretty inefficient, but there aren't enough initialGates to really care
	int waiting = 1;
	int detectLoop = 0;
	while (waiting && !detectLoop)
	{
		detectLoop = 1; // need to make sure we make some progress, otherwise we got into a loop
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

					gate->computed = 1;
					detectLoop = 0;
				}
				else
				{
					waiting = 1;
				}
			}
		}
	}

	return !detectLoop;
}

void initInputRegisters(struct HashTable* registerTable, long long number, int numRegisters, char registerChar)
{
	for (int registerIndex = 0; registerIndex < numRegisters; registerIndex++)
	{
		int bit = (number & (1LL << registerIndex)) >> registerIndex;

		char registerName[4];
		sprintf(registerName, "%c%02d", registerChar, registerIndex);
		int* value = hasPatternHashTable(registerTable, registerName);
		assert(value != NULL);
		*value = bit;
	}
}

long long findOutput(struct HashTable* registerTable, int numOutputBits)
{
	// now find our secret number
	long long output = 0;
	for (int outputIndex = 0; outputIndex < numOutputBits; outputIndex++)
	{
		char outputRegister[4];
		sprintf(outputRegister, "z%02d", outputIndex);
		int* outputValue = hasPatternHashTable(registerTable, outputRegister);
		assert(outputValue != NULL);
		if (outputValue == NULL || (*outputValue != 0 && *outputValue != 1)) // verify that we have a valid result
		{
			//return -1;
			*outputValue = 0;
		}
		output |= ((long long)(*outputValue)) << outputIndex; // set the specified bit to the value
	}
	return output;
}

int verifyGraphCorrectness(struct Gate* gates, int numGates, const struct HashTable* registerTable, struct HashTable* verifyTable, int bitIndex)
{
	int verified = 1;
	struct Gate* verifyGates = malloc(sizeof(struct Gate) * numGates);
	{
		memcpy(verifyGates, gates, sizeof(struct Gate) * numGates);
		long long testBit = 1LL << (bitIndex - 1);
		memcpy(verifyTable, registerTable, sizeof(struct HashTable));

		initInputRegisters(verifyTable, testBit, bitIndex, 'x');
		initInputRegisters(verifyTable, testBit, bitIndex, 'y');

		int validResult = computeGates(verifyGates, numGates, verifyTable);
		long long verifyOutput = validResult ? findOutput(verifyTable, bitIndex + 1) : -1;
		if (!validResult || verifyOutput != testBit + testBit)
		{
			verified = 0;
		}
	}
	free(verifyGates);
	return verified;
}

void swapStrings(char* s0, char* s1)
{
	char temp[4];
	strcpy(temp, s0);
	strcpy(s0, s1);
	strcpy(s1, temp);
}

void alphaSortStrings(char** strings, int numStrings)
{
	// just bubble sort these, they will be few in number
	for (int i0 = 0; i0 < numStrings; i0++)
	{
		for (int i1 = i0 + 1; i1 < numStrings; i1++)
		{
			if (strcmp(strings[i0], strings[i1]) > 0)
			{
				swapStrings(strings[i0], strings[i1]);
			}
		}
	}
}

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	struct HashTable* registerTable = malloc(sizeof(struct HashTable));
	initHashTable(registerTable);

#define MAX_GATES 256
	struct Gate* initialGates = malloc(sizeof(struct Gate) * MAX_GATES);
	int numGates = 0;

	// we want to keep track of the number of final output registers so we can construct our number at the end
	int numFinalOutputRegisters = 0;

	int numXInputRegisters = 0; // just verifying range
	int numYInputRegisters = 0;

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
			addHashTable(registerTable, name0, /*value*/0); // set to 0 since we are fixing this so will provide our own inputs

			if (name0[0] == 'x') { numXInputRegisters++; }
			if (name0[0] == 'y') { numYInputRegisters++; }
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

			assert(output[0] != 'x' && output[0] != 'y');

			initGate(&initialGates[numGates], name0, name1, operation, output);
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
	assert(numXInputRegisters == numYInputRegisters && numXInputRegisters + 1 == numFinalOutputRegisters);

#define MAX_SWAPPED 8
	char* swappedOuputPairNames[MAX_SWAPPED] = { 0 };
	for (int stringIndex = 0; stringIndex < MAX_SWAPPED; stringIndex++)
	{
		swappedOuputPairNames[stringIndex] = malloc(sizeof(char) * 4);
	}
	int numSwapped = 0;

	// if we are building an adder, we don't want XOR on any operations that don't involve input/output pins
	// so let's find those (not 100% sure on why this is, but XOR in the middle causes problems)
	for (int gateIndex = 0; gateIndex < numGates; gateIndex++)
	{
		struct Gate* gate = &initialGates[gateIndex];
		if (strcmp(gate->operation, "XOR") == 0 &&
			gate->name0[0] != 'x' && gate->name0[0] != 'y' &&
			gate->name1[0] != 'x' && gate->name1[1] != 'y' &&
			gate->output[0] != 'z')
		{
			strcpy(swappedOuputPairNames[numSwapped], gate->output);
			numSwapped++;
		}
	}

	// to fix these, we'll need to find the output they were supposed to go to
	int numInitialSwapped = numSwapped;
	int foundPair[MAX_SWAPPED] = { 0 };

	// if we are building an adder, then we'll need z00 to somehow map back to x00 and y00,
	// and so on up to the last output bit, which we can use to try and narrow down the problematic areas
	// we can technically get away with only testing the carry bit by making a few assumptions (that at least work for this puzzle)
	struct HashTable* verifyTable = malloc(sizeof(struct HashTable));
	int testSwapIndex = 0;
	struct Gate* testSwapGate0 = NULL;
	struct Gate* testSwapGate1 = NULL;
	int badBits[MAX_GATES] = { 0 };
	int testInputSwap = 0;
	for (int bitIndex = 1; bitIndex < numFinalOutputRegisters; bitIndex++)
	{
		struct Gate verifyGates[MAX_GATES];
		int numVerifyGates = 0;

		char outputRegister[4];
		sprintf(outputRegister, "z%02d", bitIndex);
		buildGateGraphByOutput(initialGates, numGates, outputRegister, verifyGates, &numVerifyGates);

		// see if this is correct
		int verified = verifyGraphCorrectness(verifyGates, numVerifyGates, registerTable, verifyTable, bitIndex);
		if (verified)
		{
			// keep any working swaps
			if (testSwapGate0 != NULL && testSwapGate1 != NULL)
			{
				if (!testInputSwap)
				{
					foundPair[testSwapIndex] = 1;
				}
				else
				{
					strcpy(swappedOuputPairNames[numSwapped], testSwapGate0->output);
					numSwapped++;
					strcpy(swappedOuputPairNames[numSwapped], testSwapGate1->output);
					numSwapped++;
				}
				testSwapGate0 = NULL;
				testSwapGate1 = NULL;
				testSwapIndex = 0;
				testInputSwap = 0;
			}
		}
		else
		{
			// see if swapping with any of our previously bad gates work
			if (testSwapGate0 != NULL && testSwapGate1 != NULL)
			{
				// this didn't work, so change it back
				swapStrings(testSwapGate0->output, testSwapGate1->output);
				numSwapped--;
				testSwapIndex++;
			}

			while (foundPair[testSwapIndex] && testSwapIndex < numInitialSwapped) { testSwapIndex++; }

			if (testSwapIndex < numInitialSwapped)
			{
				// our later bit was bad, so it's something in the carry logic, so let's try swapping the output of that
				char outputRegister[4];
				sprintf(outputRegister, "z%02d", bitIndex);
				testSwapGate0 = findGateByOutput(initialGates, numGates, outputRegister);
				testSwapGate1 = findGateByOutput(initialGates, numGates, swappedOuputPairNames[testSwapIndex]);

				strcpy(swappedOuputPairNames[numSwapped], testSwapGate0->output);
				numSwapped++;
				swapStrings(testSwapGate0->output, testSwapGate1->output); // test with swap
				bitIndex--; // decrement this to try again
			}
			else
			{
				testSwapGate0 = NULL;
				testSwapGate1 = NULL;
				testSwapIndex = 0;
				if (!testInputSwap)
				{
					// it's possible this is an issue with the prior bit, so let's find those inputs and swap the outputs
					char inputRegister[4];
					sprintf(inputRegister, "x%02d", bitIndex - 1);
					struct Gate* inputGates[8];
					int numInputGates = 0;
					findGatesByInput(initialGates, numGates, inputRegister, inputGates, &numInputGates, 8);
					assert(numInputGates == 2); // should only have two of these
					testSwapGate0 = inputGates[0];
					testSwapGate1 = inputGates[1];
					swapStrings(testSwapGate0->output, testSwapGate1->output);

					testInputSwap = 1;
					bitIndex--;
				}
			}
		}
	}

	alphaSortStrings(swappedOuputPairNames, numSwapped);

	int outputIndex = 0;
	for (int swapIndex = 0; swapIndex < numSwapped; swapIndex++)
	{
		if (outputIndex > 0)
		{
			input[outputIndex] = ',';
			outputIndex++;
		}
		strcpy(&input[outputIndex], swappedOuputPairNames[swapIndex]);
		size_t length = strlen(swappedOuputPairNames[swapIndex]);
		outputIndex += (int)length;
	}

	printf("%s\n", input);
}