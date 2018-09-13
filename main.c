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

enum regs {
	r1 = 1, r2, r3, r4, r5, r6, r7, r8, r9, r10
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
		printf("%s\n", token);
		decode(token, n);
		if (token = strtok(NULL, ",\n")) {
			strcpy(mem_code[n].target, token);
			if (token = strtok(NULL, "\n")) {
				strcpy(mem_code[n].source, token);
			}
		}
		n++;
	} while (!feof(file) && n < MEMSZ);

	printf("%d\n", n);

	for (int i = 0; i < n; i++) {
		printf("OP: %d, TARGET: %s, SOURCE: %s\n", mem_code[i].instruction, mem_code[i].target, mem_code[i].source);
	}
	// printf("%d\n", mem_code[0].instruction);
	fclose(file);
	return 0;
}
