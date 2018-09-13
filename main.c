#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MEMSZ 1024
#define r0 0

unsigned registers[10];
unsigned mem_data[MEMSZ];

enum opcode {
	ADD = 1, SUB, DIV, MUL, BRZ, BRNZ, IBRZ, BLZ, MOV, LOADM, STOREM,
	LOADI, WRITE, READ, EXIT
};

struct {
	int instruction;
	char target[32], source[32];
} mem_code[MEMSZ];

void decode(char *op, int n) {
	if (!strcmp(op, "ADD"))
		mem_code[n].instruction = ADD;
	else if (!strcmp(op, "SUB"))
		mem_code[n].instruction = SUB;
	else if (!strcmp(op, "DIV"))
		mem_code[n].instruction = DIV;
	else if (!strcmp(op, "MUL"))
		mem_code[n].instruction = MUL;
	else if (!strcmp(op, "BRZ"))
		mem_code[n].instruction = BRZ;
	else if (!strcmp(op, "BRNZ"))
		mem_code[n].instruction = BRNZ;
	else if (!strcmp(op, "IBRZ"))
		mem_code[n].instruction = IBRZ;
	else if (!strcmp(op, "BLZ"))
		mem_code[n].instruction = BLZ;
	else if (!strcmp(op, "MOV"))
		mem_code[n].instruction = MOV;
	else if (!strcmp(op, "LOADM"))
		mem_code[n].instruction = LOADM;
	else if (!strcmp(op, "STOREM"))
		mem_code[n].instruction = STOREM;
	else if (!strcmp(op, "LOADI"))
		mem_code[n].instruction = LOADI;
	else if (!strcmp(op, "WRITE"))
		mem_code[n].instruction = WRITE;
	else if (!strcmp(op, "READ"))
		mem_code[n].instruction = READ;
	else if (!strcmp(op, "EXIT"))
		mem_code[n].instruction = EXIT;
	else {
		printf("Error: invalid instruction '%s'.", op);
		exit(0);
	}
}

void execute(int n) {
	int counter = 0;
	int target, source;

	for(int pc = 0; pc < n; pc++) {
		switch (mem_code[pc].instruction) {
			case ADD: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source+2);
				registers[target] = registers[target] + registers[source];
				break;
			}
			case SUB: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source+2);
				registers[target] = registers[target] - registers[source];
				break;
			}
			case DIV: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source+2);
				registers[target] = registers[target] / registers[source];
				break;
			}
			case MUL: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source+2);
				registers[target] = registers[target] * registers[source];
				break;
			}
			case BRZ: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source);
				if(!registers[target]) pc += source-1;
				break;
			}
			case BRNZ: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source);
				if(registers[target]) pc += source-1;
				break;
			}
			case IBRZ: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source);
				if(!registers[target]) pc = source-1;
				break;
			}
			case BLZ: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source);
				if(registers[target] < 0) pc += source-1;
				break;
			}
			case MOV: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source+2);
				registers[target] = registers[source];
				break;
			}
			case LOADM: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source);
				registers[target] = mem_data[source];
				break;
			}
			case STOREM: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source);
				mem_data[source] = registers[target];
				break;
			}
			case LOADI: {
				target = atoi(mem_code[pc].target+1);
				source = atoi(mem_code[pc].source);
				registers[target] = source;
				break;
			}
			case WRITE: {
				printf("%d\n", registers[atoi(mem_code[pc].target+1)]);
				break;
			}
			case READ: {
				scanf("%d", &registers[atoi(mem_code[pc].target+1)]);
				break;
			}
			case EXIT: {
				printf("Number of instructions: %d\n", counter);
				exit(0);
			}
		} //end switch
		counter++;
	}
}

int main(int argc, char **argv) {
	FILE *file;
	char buffer[255], *token;
	int n = 0;

	if ((file = fopen(argv[1], "r")) == NULL) {
		printf("Error: opening file.\n");
		exit(0);
	}

	do {
		fgets(buffer, 255, file);
		token = strtok(buffer, " \n");
		decode(token, n);
		if (token = strtok(NULL, ",\n")) {
			strcpy(mem_code[n].target, token);
			if (token = strtok(NULL, "\n")) {
				strcpy(mem_code[n].source, token);
			}
		}
		n++;
	} while (!feof(file) && n < MEMSZ);

	fclose(file);

	execute(n);

	return 0;
}
