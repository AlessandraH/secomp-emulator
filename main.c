// Headers required by LLVM
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMSZ      1024
#define THRESHOULD 50

// LLVM Stuff
LLVMValueRef ptr_register[11];
LLVMValueRef ptr_pc;

int registers[11];
int mem_data[MEMSZ];

enum opcode {
    ADD = 1,
    SUB,
    DIV,
    MUL,
    MOV,
    BRZ,
    BRNZ,
    IBRZ,
    BLZ,
    LOADM,
    STOREM,
    LOADI,
    WRITE,
    READ,
    EXIT
};

struct region {
    LLVMModuleRef          mod;
    LLVMExecutionEngineRef engine;

    LLVMValueRef           fac;
    LLVMBasicBlockRef      entry;
    LLVMBuilderRef         builder;

    bool                   done;
    void                   (* fun)(void);
};

struct {
    int             instruction, target, source;
    struct region * region;
} mem_code[MEMSZ];

void
print_regs()
{
    printf("registers[ ");
    for (int i = 0; i < 11; i++) {
        printf("%d ", registers[i]);
    }
    printf("]\n");
}

void
decode(char * op, int n)
{
    if (!strcmp(op, "ADD")) {
        mem_code[n].instruction = ADD;
    } else if (!strcmp(op, "SUB")) {
        mem_code[n].instruction = SUB;
    } else if (!strcmp(op, "DIV")) {
        mem_code[n].instruction = DIV;
    } else if (!strcmp(op, "MUL")) {
        mem_code[n].instruction = MUL;
    } else if (!strcmp(op, "BRZ")) {
        mem_code[n].instruction = BRZ;
    } else if (!strcmp(op, "BRNZ")) {
        mem_code[n].instruction = BRNZ;
    } else if (!strcmp(op, "IBRZ")) {
        mem_code[n].instruction = IBRZ;
    } else if (!strcmp(op, "BLZ")) {
        mem_code[n].instruction = BLZ;
    } else if (!strcmp(op, "MOV")) {
        mem_code[n].instruction = MOV;
    } else if (!strcmp(op, "LOADM")) {
        mem_code[n].instruction = LOADM;
    } else if (!strcmp(op, "STOREM")) {
        mem_code[n].instruction = STOREM;
    } else if (!strcmp(op, "LOADI")) {
        mem_code[n].instruction = LOADI;
    } else if (!strcmp(op, "WRITE")) {
        mem_code[n].instruction = WRITE;
    } else if (!strcmp(op, "READ")) {
        mem_code[n].instruction = READ;
    } else if (!strcmp(op, "EXIT")) {
        mem_code[n].instruction = EXIT;
    } else {
        printf("Error: invalid instruction '%s'.", op);
        exit(0);
    }
} /* decode */

bool recording = false;
struct region * actual;

void
start_region()
{
    actual       = (struct region *) malloc(sizeof(struct region));
    actual->done = false;

    actual->mod = LLVMModuleCreateWithName("toy");

    LLVMCreateJITCompilerForModule(&actual->engine, actual->mod, 2, NULL);
    LLVMTypeRef args[] = { };

    actual->fac = LLVMAddFunction(actual->mod, "fac",
        LLVMFunctionType(LLVMVoidType(), args, 0, 0));

    actual->builder = LLVMCreateBuilder();

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(actual->fac, "entry");
    actual->entry = LLVMAppendBasicBlock(actual->fac, "for");

    LLVMPositionBuilderAtEnd(actual->builder, entry);
    LLVMBuildBr(actual->builder, actual->entry);

    LLVMPositionBuilderAtEnd(actual->builder, actual->entry);

    LLVMSetFunctionCallConv(actual->fac, LLVMCCallConv);

    recording = true;
}

void
end_region(int pc)
{
    if (actual != NULL) {
        actual->done = true;

        LLVMValueRef p_pc = LLVMBuildPtrToInt(actual->builder, ptr_pc, LLVMPointerType(
                LLVMInt64Type(), 0),
            "ptr_pc");

        LLVMValueRef new_pc = LLVMConstInt(LLVMInt64Type(), pc, 0);
        LLVMBuildStore(actual->builder, new_pc, p_pc);
        LLVMBuildRet(actual->builder, NULL);

        LLVMVerifyModule(actual->mod, LLVMAbortProcessAction, NULL);
        LLVMDumpModule(actual->mod);

        actual->fun = (void (*)(void))LLVMGetFunctionAddress(actual->engine,
            "fac");

        recording = false;
    }
}

bool
call_region(int pc)
{
    if (mem_code[pc].region != NULL && mem_code[pc].region->done) {
        mem_code[pc].region->fun();

        return true;
    }

    if (recording) {
        mem_code[pc].region = actual;

        switch (mem_code[pc].instruction) {
            case ADD: {
                LLVMValueRef ptr_target = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].target], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_target");
                LLVMValueRef ptr_source = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].source], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_source");

                LLVMValueRef source = LLVMBuildLoad(actual->builder, ptr_source,
                    "source");
                LLVMValueRef target = LLVMBuildLoad(actual->builder, ptr_target,
                    "target");

                LLVMValueRef tmp = LLVMBuildAdd(actual->builder, source, target,
                    "tmp");

                LLVMBuildStore(actual->builder, tmp, ptr_target);
                break;
            }
            case SUB: {
                LLVMValueRef ptr_target = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].target], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_target");
                LLVMValueRef ptr_source = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].source], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_source");

                LLVMValueRef source = LLVMBuildLoad(actual->builder, ptr_source,
                    "source");
                LLVMValueRef target = LLVMBuildLoad(actual->builder, ptr_target,
                    "target");

                LLVMValueRef tmp = LLVMBuildSub(actual->builder, target, source,
                    "tmp");

                LLVMBuildStore(actual->builder, tmp, ptr_target);
                break;
            }
            case DIV: {
                LLVMValueRef ptr_target = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].target], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_target");
                LLVMValueRef ptr_source = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].source], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_source");

                LLVMValueRef source = LLVMBuildLoad(actual->builder, ptr_source,
                    "source");
                LLVMValueRef target = LLVMBuildLoad(actual->builder, ptr_target,
                    "target");

                LLVMValueRef tmp = LLVMBuildUDiv(actual->builder, source, target,
                    "tmp");

                LLVMBuildStore(actual->builder, tmp, ptr_target);
                break;
            }
            case MUL: {
                LLVMValueRef ptr_target = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].target], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_target");
                LLVMValueRef ptr_source = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].source], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_source");

                LLVMValueRef source = LLVMBuildLoad(actual->builder, ptr_source,
                    "source");
                LLVMValueRef target = LLVMBuildLoad(actual->builder, ptr_target,
                    "target");

                LLVMValueRef tmp = LLVMBuildMul(actual->builder, source, target,
                    "tmp");

                LLVMBuildStore(actual->builder, tmp, ptr_target);
                break;
            }
            case BRZ: {
                break;
            }
            case BRNZ: {
                LLVMBasicBlockRef cond_br = LLVMAppendBasicBlock(actual->fac, "cond");
                LLVMBasicBlockRef then = LLVMAppendBasicBlock(actual->fac, "then");
                LLVMBasicBlockRef else_br = LLVMAppendBasicBlock(actual->fac, "else");
                LLVMBasicBlockRef exit  = LLVMAppendBasicBlock(actual->fac, "exit");

                LLVMBuildBr(actual->builder, cond_br);

                LLVMPositionBuilderAtEnd(actual->builder, cond_br);
                LLVMValueRef ptr_target = LLVMBuildPtrToInt(actual->builder,
                    ptr_register[mem_code[pc
                    ].target], LLVMPointerType(
                        LLVMInt64Type(), 0),
                    "ptr_target");
                LLVMValueRef target = LLVMBuildLoad(actual->builder, ptr_target,
                    "target");

                LLVMBuildCondBr(actual->builder,
                  LLVMBuildICmp(actual->builder, LLVMIntNE, target,
                  LLVMConstInt(LLVMInt64Type(), 0, 0),
                  "cond"), then, else_br);

                LLVMPositionBuilderAtEnd(actual->builder, then);
                LLVMBuildBr(actual->builder, actual->entry);

                LLVMPositionBuilderAtEnd(actual->builder, else_br);
                LLVMBuildBr(actual->builder, exit);

                LLVMPositionBuilderAtEnd(actual->builder, exit);
                break;
            }
            case IBRZ: {
                break;
            }
            case BLZ: {
                break;
            }
            case MOV: {
                break;
            }
            case LOADM: {
                break;
            }
            case STOREM: {
                break;
            }
            case LOADI: {
                break;
            }
            case WRITE: {
                break;
            }
            case READ: {
                break;
            }
            case EXIT: {
                exit(0);
            }
        } // end switch
    }
    return false;
} /* call_region */

void
execute(int n)
{
    int counter = 0;
    int hot_code[n];
    unsigned int pc;

    // Initialize ptr contants to registers
    ptr_pc = LLVMConstInt(LLVMInt64Type(), (long long) &pc, 0);

    for (pc = 0; pc < n; pc++) {
        if (!call_region(pc)) {
            switch (mem_code[pc].instruction) {
                case ADD: {
                    registers[mem_code[pc].target] +=
                      registers[mem_code[pc].source];
                    break;
                }
                case SUB: {
                    registers[mem_code[pc].target] -=
                      registers[mem_code[pc].source];
                    break;
                }
                case DIV: {
                    registers[mem_code[pc].target] /=
                      registers[mem_code[pc].source];
                    break;
                }
                case MUL: {
                    registers[mem_code[pc].target] *=
                      registers[mem_code[pc].source];
                    break;
                }
                case BRZ: {
                    end_region(pc);
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
                    end_region(pc);
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
                    end_region(pc);
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
                    end_region(pc);
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
            } // end switch
        }
        counter++;
    }
} /* execute */

void
init_llvm()
{
    // Start LLVM JIT
    LLVMLinkInMCJIT();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();

    for (int i = 0; i < 11; i++) {
        ptr_register[i] = LLVMConstInt(
            LLVMInt64Type(), (long long) &registers[i], 0);
    }
}

int
main(int _argc, char ** argv)
{
    init_llvm();

    FILE * file;
    char buffer[255], * token;
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
} /* main */
