#ifndef VM_H__HEADER_GUARD__
#define VM_H__HEADER_GUARD__

/* TODO: probably split into more files */

#include <stdint.h>  /* uint8_t, uint16_t, uint32_t, uint64_t */
#include <stdio.h>   /* fprintf, stdout, stdin, stderr, fputs, fputc, fwrite, fflush,
                        FILE, fopen, fclose, fgetc, fread */
#include <stdbool.h> /* bool, true, false */
#include <stdlib.h>  /* size_t, exit, EXIT_FAILURE, free */
#include <string.h>  /* memset, memcpy */
#include <assert.h>  /* assert */

#include "colors.h"
#include "error.h"
#include "utils.h"

/* in bytes */
#define STACK_SIZE 0xFFFF
#define INST_SIZE  10

#define GEN_REGS_COUNT   15
#define OPEN_FILES_LIMIT 128

#define ERR_PREFIX "[ERROR] "

typedef uint64_t word_t;

/* TODO: conditional moves, writes and reads */

typedef enum {
/*x*/	OPCODE_NONE = 0, /* nothing */

/*X*/	OPCODE_MOVE,   /* move constant value into R1 */
/*X*/	OPCODE_MOVE_R, /* move value from R2 to R1 */

/*X*/	OPCODE_WRITE_64,   /* write constant value to address in R1 (64 bit) */
/*X*/	OPCODE_WRITE_R_64, /* write value from R2 to address in R1  (64 bit) */

/*X*/	OPCODE_WRITE_32,   /* write constant value to address in R1 (32 bit) */
/*X*/	OPCODE_WRITE_R_32, /* write value from R2 to address in R1  (32 bit) */

/*X*/	OPCODE_WRITE_16,   /* write constant value to address in R1 (16 bit) */
/*X*/	OPCODE_WRITE_R_16, /* write value from R2 to address in R1  (16 bit) */

/*X*/	OPCODE_WRITE_8,   /* write constant value to address in R1 (8 bit) */
/*X*/	OPCODE_WRITE_R_8, /* write value from R2 to address in R1  (8 bit) */

/*X*/	OPCODE_READ_64,   /* read value from address to R1       (64 bit) */
/*X*/	OPCODE_READ_R_64, /* read value from address in R2 to R1 (64 bit) */

/*X*/	OPCODE_READ_32,   /* read value from address to R1       (32 bit) */
/*X*/	OPCODE_READ_R_32, /* read value from address in R2 to R1 (32 bit) */

/*X*/	OPCODE_READ_16,   /* read value from address to R1       (16 bit) */
/*X*/	OPCODE_READ_R_16, /* read value from address in R2 to R1 (16 bit) */

/*X*/	OPCODE_READ_8,   /* read value from address to R1       (8 bit) */
/*X*/	OPCODE_READ_R_8, /* read value from address in R2 to R1 (8 bit) */

/*x*/	OPCODE_PUSH_64,   /* push a constant value on the stack (64 bit) */
/*x*/	OPCODE_PUSH_R_64, /* push R1 to the stack               (64 bit) */

/*x*/	OPCODE_PUSH_32,   /* push a constant value on the stack (32 bit) */
/*x*/	OPCODE_PUSH_R_32, /* push R1 to the stack               (32 bit) */

/*x*/	OPCODE_PUSH_16,   /* push a constant value on the stack (16 bit) */
/*x*/	OPCODE_PUSH_R_16, /* push R1 to the stack               (16 bit) */

/*x*/	OPCODE_PUSH_8,   /* push a constant value on the stack (8 bit) */
/*x*/	OPCODE_PUSH_R_8, /* push R1 to the stack               (8 bit) */

/*x*/	OPCODE_PUSH_A, /* push all general purpose registers to the stack */

/*x*/	OPCODE_POP_64,   /* pop a value off the stack         (64 bit)*/
/*x*/	OPCODE_POP_R_64, /* pop a value off the stack into R1 (64 bit)*/

/*x*/	OPCODE_POP_32,   /* pop a value off the stack         (32 bit)*/
/*x*/	OPCODE_POP_R_32, /* pop a value off the stack into R1 (32 bit)*/

/*x*/	OPCODE_POP_16,   /* pop a value off the stack         (16 bit)*/
/*x*/	OPCODE_POP_R_16, /* pop a value off the stack into R1 (16 bit)*/

/*x*/	OPCODE_POP_8,   /* pop a value off the stack         (8 bit)*/
/*x*/	OPCODE_POP_R_8, /* pop a value off the stack into R1 (8 bit)*/

/*x*/	OPCODE_POP_A, /* pop values back into all general purpose registers */

/*x*/	OPCODE_EQ,    /* R1 is equal to constant value */
/*x*/	OPCODE_EQ_R,  /* R1 is equal to R2 */
/*x*/	OPCODE_NEQ,   /* R1 is not equal to constant value */
/*x*/	OPCODE_NEQ_R, /* R1 is not equal to R2 */
/*x*/	OPCODE_GT,    /* R1 is greater than constant value */
/*x*/	OPCODE_GT_R,  /* R1 is greater than R2 */
/*x*/	OPCODE_GE,    /* R1 is greater or equal to constant value */
/*x*/	OPCODE_GE_R,  /* R1 is greater or equal to R2 */
/*x*/	OPCODE_LT,    /* R1 is less than constant value */
/*x*/	OPCODE_LT_R,  /* R1 is less than R2 */
/*x*/	OPCODE_LE,    /* R1 is less or equal to constant value */
/*x*/	OPCODE_LE_R,  /* R1 is less or equal to R2 */

/*x*/	OPCODE_JUMP,     /* jump */
/*x*/	OPCODE_JUMP_R,   /* jump to address in R1 */
/*x*/	OPCODE_JUMP_T,   /* jump if true */
/*x*/	OPCODE_JUMP_T_R, /* jump if true to address in R1 */
/*x*/	OPCODE_JUMP_F,   /* jump if false */
/*x*/	OPCODE_JUMP_F_R, /* jump if false to address in R1 */

/*x*/	OPCODE_ADD,     /* add a constant value to R1 (unsigned) */
/*x*/	OPCODE_ADD_R,   /* add R2 to R1 (unsigned) */
	OPCODE_ADD_S,   /* add a constant value to R1 (signed) */
	OPCODE_ADD_R_S, /* add R2 to R1 (signed) */
/*x*/	OPCODE_INC,     /* increment R1 by 1 */

/*x*/	OPCODE_SUB,     /* subtract a constant value from R1 (unsigned) */
/*x*/	OPCODE_SUB_R,   /* subtract R2 from R1 (unsigned) */
	OPCODE_SUB_S,   /* subtract a constant value from R1 (signed) */
	OPCODE_SUB_R_S, /* subtract R2 from R1 (signed) */
/*x*/	OPCODE_DEC,     /* decrement R1 by 1 */

/*x*/	OPCODE_MULT,     /* multiply R1 by a constant value (unsigned) */
/*x*/	OPCODE_MULT_R,   /* multiply R1 by R2 (unsigned) */
	OPCODE_MULT_S,   /* multiply R1 by a constant value (signed) */
	OPCODE_MULT_R_S, /* multiply R1 by R2 (signed) */

/*x*/	OPCODE_DIV,     /* divide R1 by a constant value (unsigned) */
/*x*/	OPCODE_DIV_R,   /* divide R1 by R2 (unsigned) */
	OPCODE_DIV_S,   /* divide R1 by a constant value (signed) */
	OPCODE_DIV_R_S, /* divide R1 by R2 (signed) */

/*x*/	OPCODE_MOD,     /* modulus of R1 and a constant value, stored in R1 (unsigned) */
/*x*/	OPCODE_MOD_R,   /* modulus of R1 and R2, stored in R1 (unsigned) */
	OPCODE_MOD_S,   /* modulus of R1 and a constant value, stored in R1 (unsigned) */
	OPCODE_MOD_R_S, /* modulus of R1 and R2, stored in R1 (unsigned) */

	OPCODE_RSHIFT,   /* shift R1 to the right by constant value bits */
	OPCODE_RSHIFT_R, /* shift R1 to the right by R2 bits */
	OPCODE_LSHIFT,   /* shift R1 to the left by constant value bits */
	OPCODE_LSHIFT_R, /* shift R1 to the left by R2 bits */

	OPCODE_AND, /* R1 and R2, result stored in CN */
	OPCODE_OR,  /* R1 or R2, result stored in CN */
	OPCODE_NOT, /* not R1, result stored in CN */

	OPCODE_BITAND, /* R1 bit and R2, result stored in R1 */
	OPCODE_BITOR,  /* R1 bit or R2, result stored in R1 */

/*x*/	OPCODE_CALL,     /* jump, saving the previous ip */
/*x*/	OPCODE_CALL_R,   /* jump to address in R1, saving the previous ip */
/*x*/	OPCODE_CALL_T,   /* jump if true, saving the previous ip */
/*x*/	OPCODE_CALL_T_R, /* jump if true to address in R1, saving the previous ip */
/*x*/	OPCODE_CALL_F,   /* jump if false, saving the previous ip */
/*x*/	OPCODE_CALL_F_R, /* jump if false to address in R1, saving the previous ip */
/*x*/	OPCODE_RET,      /* return to the saved instruction position */

/*x*/	OPCODE_SYSCALL,   /* call a built-in system function by the id */
/*x*/	OPCODE_SYSCALL_R, /* call a built-in system function by the id in R1 */

/*x*/	OPCODE_HALT = 0xFF /* halt the vm */
} opcode_t;

typedef enum {
	REG_1 = 0, /* general purpose registers */
	REG_2,
	REG_3,
	REG_4,
	REG_5,
	REG_6,
	REG_7,
	REG_8,
	REG_9,
	REG_10,
	REG_11,
	REG_12,
	REG_13,
	REG_14,
	REG_15,

	REG_AC, /* accumulator register */
	REG_IP, /* instruction pointer */
	REG_SP, /* stack pointer */
	REG_SB, /* stack base pointer */
	REG_CN, /* conditional register */
	REG_EX, /* exitcode register */
	REGS_COUNT
} reg_t;

typedef enum {
	ERR_STACK_OVERFLOW = 0,
	ERR_STACK_UNDERFLOW,
	ERR_UNKNOWN_INSTRUCTION,
	ERR_INVALID_ACCESS,
	ERR_NO_MORE_FILES,
	ERR_WRITE_TO_READ_ONLY,
	ERR_DIV_BY_ZERO
} err_t;

typedef enum {
	SYSCALL_WRITEF = 0, /* write bytes at memory with a given size to a file */
	SYSCALL_READF,      /* read bytes from a file with a given size to memory */

	SYSCALL_MEMSET,  /* set a region of memory to a certain value */
	SYSCALL_MEMCOPY, /* copy a region of memory to another */

	SYSCALL_FLUSH, /* flush a stream */

	SYSCALL_OPENF,  /* open a file */
	SYSCALL_CLOSEF, /* close a file */

	SYSCALL_DEBUG /* a function to dumb the stack (temporary) */
} syscall_t;

typedef struct {
	opcode_t opcode: 8; /* making sure these values are EXACTLY 8 bits */
	reg_t    reg:    8;
	word_t   data;
} inst_t;

typedef struct {
	bool  reading;
	FILE *file;
} stream_t;

typedef struct {
	/* little endian is used */

	/* static_memory is segmented into the data segment and stack */
	uint8_t *static_memory;
	word_t   stack_start, stack_end;

	inst_t *program;
	word_t  program_size;

	uint8_t *data_segment;
	word_t   data_segment_size;

	stream_t open_files[OPEN_FILES_LIMIT];

	word_t  regs[REGS_COUNT];
	word_t *ip, *sp;

	bool halt;
} vm_t;

void vm_init(vm_t *p_vm);
void vm_load_from_file(vm_t *p_vm, const char *p_path);
int  vm_exec(vm_t *p_vm);

void vm_dump(vm_t *p_vm, FILE *p_stream); /* debug */
void vm_panic(vm_t *p_vm, err_t p_err);

void vm_exec_inst(vm_t *p_vm, inst_t *p_inst);

word_t  *vm_access_reg(vm_t *p_vm, reg_t p_reg);
uint8_t *vm_access_mem(vm_t *p_vm, word_t p_addr, char p_for);

uint8_t  vm_read8(vm_t *p_vm, word_t p_addr);
uint16_t vm_read16(vm_t *p_vm, word_t p_addr);
uint32_t vm_read32(vm_t *p_vm, word_t p_addr);
uint64_t vm_read64(vm_t *p_vm, word_t p_addr);

void vm_write8(vm_t *p_vm, word_t p_addr, uint8_t p_data);
void vm_write16(vm_t *p_vm, word_t p_addr, uint16_t p_data);
void vm_write32(vm_t *p_vm, word_t p_addr, uint32_t p_data);
void vm_write64(vm_t *p_vm, word_t p_addr, uint64_t p_data);

#endif
