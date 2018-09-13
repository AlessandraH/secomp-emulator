#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MEMSZ 1024

unsigned registers[11];
unsigned mem_data[MEMSZ];

enum opcode {
	ADD = 1, SUB, DIV, MUL, MOV, //1
	BRZ, BRNZ, IBRZ, BLZ, LOADM, STOREM, LOADI, //6
	WRITE, READ, EXIT //13
};

struct {
	int instruction, target, source;
} mem_code[MEMSZ];

void print_regs() {
	printf("registers[ ");
	for (int i = 0; i <11; i++) {
		printf("%d ", registers[i]);
	}
	printf("]\n");
}

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

	for(int pc = 0; pc < n; pc++) {
		switch (mem_code[pc].instruction) {
			case ADD: {
				registers[mem_code[pc].target] = registers[mem_code[pc].target] + registers[mem_code[pc].source];
				break;
			}
			case SUB: {
				registers[mem_code[pc].target] = registers[mem_code[pc].target] - registers[mem_code[pc].source];
				break;
			}
			case DIV: {
				registers[mem_code[pc].target] = registers[mem_code[pc].target] / registers[mem_code[pc].source];
				break;
			}
			case MUL: {
				registers[mem_code[pc].target] = registers[mem_code[pc].target] * registers[mem_code[pc].source];
				break;
			}
			case BRZ: {
				if(!registers[mem_code[pc].target]) pc += mem_code[pc].source-1;
				break;
			}
			case BRNZ: {
				if(registers[mem_code[pc].target]) pc += mem_code[pc].source-1;
				break;
			}
			case IBRZ: {
				if(!registers[mem_code[pc].target]) pc = mem_code[pc].source-1;
				break;
			}
			case BLZ: {
				if(registers[mem_code[pc].target] < 0) pc += mem_code[pc].source-1;
				break;
			}
			case MOV: {
				registers[mem_code[pc].target] = registers[mem_code[pc].source];
				break;
			}
			case LOADM: {
				registers[mem_code[pc].target] = mem_data[mem_code[pc].source];
				break;
			}
			case STOREM: {
				mem_data[mem_code[pc].source] = registers[mem_code[pc].target];
				break;
			}
			case LOADI: {
				registers[mem_code[pc].target] = mem_code[pc].source;
				break;
			}
			case WRITE: {
				printf("%d\n", registers[mem_code[pc].target]);
				break;
			}
			case READ: {
				scanf("%d", &registers[mem_code[pc].target]);
				break;
			}
			case EXIT: {
				print_regs();
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
		if (mem_code[n].instruction < 6) {
			token = strtok(NULL, ",\n");
			mem_code[n].target = atoi(token+1);
			token = strtok(NULL, "\n");
			mem_code[n].source = atoi(token+2);
		} else if (mem_code[n].instruction < 13) {
			token = strtok(NULL, ",\n");
			mem_code[n].target = atoi(token+1);
			token = strtok(NULL, "\n");
			mem_code[n].source = atoi(token);
		} else if (mem_code[n].instruction != 15){
			token = strtok(NULL, ",\n");
			mem_code[n].target = atoi(token+1);
		}
		n++;
	} while (!feof(file) && n < MEMSZ);

	fclose(file);

	execute(n);

	return 0;
}
