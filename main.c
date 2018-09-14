// Headers required by LLVM
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <llvm-c/Core.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMSZ 1024
#define THRESHOULD 50

// LLVM Stuff
LLVMModuleRef mod;
LLVMExecutionEngineRef engine;

unsigned registers[11];
unsigned mem_data[MEMSZ];

enum opcode {
    ADD = 1,
    SUB,
    DIV,
    MUL,
    MOV, //1
    BRZ,
    BRNZ,
    IBRZ,
    BLZ,
    LOADM,
    STOREM,
    LOADI, //6
    WRITE,
    READ,
    EXIT //13
};

struct region {
    LLVMValueRef fac;
    LLVMBasicBlockRef entry;
    LLVMBuilderRef builder;
    bool done;
};

struct {
    int instruction, target, source;
    struct region* region;
} mem_code[MEMSZ];

void print_regs()
{
    printf("registers[ ");
    for (int i = 0; i < 11; i++) {
        printf("%d ", registers[i]);
    }
    printf("]\n");
}

void decode(char* op, int n)
{
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

bool recording = false;
struct region* actual;

void start_region()
{
    actual = (struct region*)malloc(sizeof(struct region));

    actual->fac = LLVMAddFunction(mod, "fac", LLVMFunctionType(LLVMVoidType(), NULL, 0, 0));
    actual->entry = LLVMAppendBasicBlock(actual->fac, "entry");
    actual->builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(actual->builder, actual->entry);

    recording = true;
}

void end_region()
{
    recording = false;
}

bool call_region(int pc)
{
    if (mem_code[pc].region->done) {
        LLVMRunFunction(engine, mem_code[pc].region->fac, 0, NULL);
        return true;
    }
    if (recording) {
        mem_code[pc].region = actual;
        switch (mem_code[pc].instruction) {
        case ADD: {
            LLVMValueRef int_ptr_target = LLVMConstInt(LLVMInt64Type(), (long long)&registers[mem_code[pc].target], 0);
            LLVMValueRef int_ptr_source = LLVMConstInt(LLVMInt64Type(), (long long)&registers[mem_code[pc].source], 0);
            LLVMValueRef ptr_target = LLVMBuildIntToPtr(actual->builder, int_ptr_target, LLVMInt32Type(), 0);
            LLVMValueRef ptr_source = LLVMBuildIntToPtr(actual->builder, int_ptr_target, LLVMInt32Type(), 0);

            LLVMValueRef tmp = LLVMBuildAdd(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].target], 0),
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].source], 0),
                "tmp");
            LLVMBuildStore(
                actual->builder,
                tmp,
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&registers[mem_code[pc].target], 0));
            break;
        }
        case SUB: {
            LLVMValueRef tmp = LLVMBuildSub(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].target], 0),
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].source], 0),
                "tmp");
            LLVMBuildStore(
                actual->builder,
                tmp,
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&registers[mem_code[pc].target], 0));
            break;
        }
        case DIV: {
            LLVMValueRef tmp = LLVMBuildUDiv(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].target], 0),
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].source], 0),
                "tmp");
            LLVMBuildStore(
                actual->builder,
                tmp,
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&registers[mem_code[pc].target], 0));
            break;
        }
        case MUL: {
            LLVMValueRef tmp = LLVMBuildMul(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].target], 0),
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].source], 0),
                "tmp");
            LLVMBuildStore(
                actual->builder,
                tmp,
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&registers[mem_code[pc].target], 0));
            break;
        }
        case BRZ: {
            end_region();
            if (!registers[mem_code[pc].target]) {
                pc += mem_code[pc].source;
                hot_code[pc]++;
                if (hot_code[pc] > THRESHOULD)
                    start_region();
                pc--;
            }
            break;
        }
        case BRNZ: {
            end_region();
            if (registers[mem_code[pc].target]) {
                pc += mem_code[pc].source;
                hot_code[pc]++;
                if (hot_code[pc] > THRESHOULD)
                    start_region();
                pc--;
            }
            break;
        }
        case IBRZ: {
            end_region();
            if (!registers[mem_code[pc].target]) {
                pc = mem_code[pc].source;
                hot_code[pc]++;
                if (hot_code[pc] > THRESHOULD)
                    start_region();
                pc--;
            }
            break;
        }
        case BLZ: {
            end_region();
            if (registers[mem_code[pc].target] < 0) {
                pc += mem_code[pc].source;
                hot_code[pc]++;
                if (hot_code[pc] > THRESHOULD)
                    start_region();
                pc--;
            }
            break;
        }
        case MOV: {
            LLVMValueRef s_llvm = LLVMConstInt(LLVMInt64Type(), &registers[mem_code[pc].source], 0);

            LLVMBuildStore(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].source], 0),
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&registers[mem_code[pc].target], 0));
            break;
        }
        case LOADM: {
            LLVMBuildStore(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), mem_data[mem_code[pc].source], 0),
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&registers[mem_code[pc].target], 0));
            break;
        }
        case STOREM: {
            LLVMBuildStore(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), registers[mem_code[pc].source], 0),
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&mem_data[mem_code[pc].target], 0));
            break;
        }
        case LOADI: {
            LLVMBuildStore(
                actual->builder,
                LLVMConstInt(LLVMInt32Type(), mem_code[pc].source, 0),
                LLVMConstInt(
                    LLVMInt64Type(),
                    (long long)&registers[mem_code[pc].target], 0));
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
            exit(0);
        }
        } //end switch
    }
    return false;
}

void execute(int n)
{
    int counter = 0;
    int hot_code[n];

    for (int pc = 0; pc < n; pc++) {
        if (!call_region(pc))
            switch (mem_code[pc].instruction) {
            case ADD: {
                registers[mem_code[pc].target] += registers[mem_code[pc].source];
                break;
            }
            case SUB: {
                registers[mem_code[pc].target] -= registers[mem_code[pc].source];
                break;
            }
            case DIV: {
                registers[mem_code[pc].target] /= registers[mem_code[pc].source];
                break;
            }
            case MUL: {
                registers[mem_code[pc].target] *= registers[mem_code[pc].source];
                break;
            }
            case BRZ: {
                end_region();
                if (!registers[mem_code[pc].target]) {
                    pc += mem_code[pc].source;
                    hot_code[pc]++;
                    if (hot_code[pc] > THRESHOULD)
                        start_region();
                    pc--;
                }
                break;
            }
            case BRNZ: {
                end_region();
                if (registers[mem_code[pc].target]) {
                    pc += mem_code[pc].source;
                    hot_code[pc]++;
                    if (hot_code[pc] > THRESHOULD)
                        start_region();
                    pc--;
                }
                break;
            }
            case IBRZ: {
                end_region();
                if (!registers[mem_code[pc].target]) {
                    pc = mem_code[pc].source;
                    hot_code[pc]++;
                    if (hot_code[pc] > THRESHOULD)
                        start_region();
                    pc--;
                }
                break;
            }
            case BLZ: {
                end_region();
                if (registers[mem_code[pc].target] < 0) {
                    pc += mem_code[pc].source;
                    hot_code[pc]++;
                    if (hot_code[pc] > THRESHOULD)
                        start_region();
                    pc--;
                }
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

int main(int argc, char** argv)
{
    FILE* file;
    char buffer[255], *token;
    int n = 0;
    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    mod = LLVMModuleCreateWithName("toy");
    LLVMCreateExecutionEngineForModule(&engine, mod, NULL);

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
            mem_code[n].target = atoi(token + 1);
            token = strtok(NULL, "\n");
            mem_code[n].source = atoi(token + 2);
        } else if (mem_code[n].instruction < 13) {
            token = strtok(NULL, ",\n");
            mem_code[n].target = atoi(token + 1);
            token = strtok(NULL, "\n");
            mem_code[n].source = atoi(token);
        } else if (mem_code[n].instruction != 15) {
            token = strtok(NULL, ",\n");
            mem_code[n].target = atoi(token + 1);
        }
        n++;
    } while (!feof(file) && n < MEMSZ);

    fclose(file);

    execute(n);

    return 0;
}
