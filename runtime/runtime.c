#include <stdio.h>
#include <string.h>
#include "../parser/main.h"
#include "../parser/parser.h"
#include "../math/math.h"
#include "functions.h"

int getVariable(char *name, struct Memory *memory) {
	for (int v = 0; v < memory->variablesLength; v++) {
		if (!strcmp(name, memory->variables[v].name)) {
			return v;
		}
	}

	return -1;
}

int setVariable(char *name, char *value, struct Memory *memory) {
	// Check if var doesn't exist
	int findVar = getVariable(name, memory);
	if (findVar == -1) {
		return -1;
	} else {
		strcpy(memory->variables[findVar].value, value);
	}
}

int createVariable(char *name, char *value, struct Memory *memory) {
	// Check if var already exists
	int tryVar = getVariable(name, memory);
	if (tryVar != -1) {
		return -1;
	}

	memory->variables[memory->variablesLength].name = name;
	strcpy(memory->variables[memory->variablesLength].value, value);
	memory->variablesLength++;
	return 0;
}

int parseIntOrVar(int *error, char *string, int length, struct Memory *memory) {
	*error = 0;
	int result = 0;

	// Check if variable exists
	int tryVar = getVariable(string, memory);
	int intError = 0;
	if (tryVar == -1) {
		result = strToInt(string, &intError, length);
	} else {
		result = strToInt(
			memory->variables[tryVar].value,
			&intError,
			strlen(memory->variables[tryVar].value)
		);
	}

	if (intError) {
		*error = -1;
	}

	return result;
}



int parseRaw(char *buffer, char *string, struct Memory *memory) {
	int findVar = getVariable(string, memory);
	if (findVar == -1) {
		memset(buffer, '\0', 50);
		int tryFunc = executeFunction(buffer, string, memory);
		//printf("%s\n", buffer);

		// Function fail code
		if (tryFunc) {
			return tryFunc;
		}
	} else {
		strcpy(buffer, memory->variables[findVar].value);
	}

	return 0;
}

void parseString(char *buffer, struct Tree *tree, struct Memory *memory) {
	for (int s = 0; s < tree->stringsLength; s++) {
		if (tree->strings[s].type == RAW) {
			char rawBuffer[50];
			int tryRaw = parseRaw(rawBuffer, tree->strings[s].value, memory);
			if (tryRaw == -1) {
				strcpy(rawBuffer, "[Unknown function or variable]");
			}

			strcat(buffer, rawBuffer);
		} else {
			strcat(buffer, tree->strings[s].value);
		}
	}
}

int findLabel(char *name, struct Memory *memory) {

	for (int l = 0; l < memory->labelsLength; l++) {
		//printf("---%s-%s\n", memory->labels[l].name, name);
		if (!strcmp(name, memory->labels[l].name)) {
			//printf("---%s-%s\n", memory->labels[l].name, name);
			return l;
		}
	}

	return -1;
}

// Function for validating and setting goto
int gotoLabel(char *name, int l, struct Memory *memory) {
	//printf("---%s-%s\n", memory->labels[2].name, name);
	int label = findLabel(name, memory);

	if (label == -1) {
		return label;
	}

	memory->labels[label].lastUsed = l;
	//printf("---%s-%d\n", name, l + 1);
	return memory->labels[label].line;
}


// TODO: Use enum for errors
int testCondition(int *error, char *operator, char *first, char *second, char l, struct Memory *memory) {
	int tryVar = getVariable(first, memory);
	if (tryVar == -1) {
		*error = -1;
		return 0;
	}

	int findColon = -1;
	for (int c = 0; c < strlen(second); c++) {
		if (second[c] == ':') {
			findColon = c;
			break;
		}
	}

	if (findColon == -1) {
		*error = -2;
	}

	// Point to string after the colon
	int labelLine = gotoLabel(second + findColon + 1, l, memory);

	// Error: Bad label
	if (labelLine == -1) {
		*error = -3;
		return 0;
	}

	char actualValue[50];
	memset(actualValue, '\0', 50);
	for (int c = 0; c < findColon; c++) {
		actualValue[c] = second[c];
	}

	//printf("%d\n", memory->variablesLength);

	int test = -1;
	switch (operator[0]) {
	case EQUAL:
		test = strcmp(memory->variables[tryVar].value, actualValue);
		break;
	default:
		*error = -4; // Invalid operator
		return 0;
	}

	if (test) {
		*error = -5;
	} else {
		//printf("---%d--\n", labelLine);
		return labelLine - 1;
	}

	return 0;
}