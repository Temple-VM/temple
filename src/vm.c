#include "vm.h"

void vm_init(vm_t *p_vm) {
	memset(p_vm, 0, sizeof(vm_t));

	p_vm->ip = &p_vm->regs[REG_IP];
	p_vm->sp = &p_vm->regs[REG_SP];

	p_vm->program = NULL;
}

void vm_load_from_file(vm_t *p_vm, const char *p_path) {
	assert(sizeof(word_t) == 8);

	if (p_vm->program != NULL)
		SMEMFREE(p_vm->program);

	FILE *file = fopen(p_path, "r");
	if (file == NULL)
		fatal("Could not open file "QUOTES("%s"), p_path);

	/* skip the shebang */
	char ch = fgetc(file);
	if (ch == '#')
		while (fgetc(file) != '\n');
	else
		ungetc(ch, file);

	char ext[3] = {0};

	int tmp = fread(ext, 1, sizeof(ext) - 1, file);
	if (tmp < (int)sizeof(ext) - 1)
		fatal(QUOTES("%s")" is not a compatible file format", p_path);

	if (strcmp(ext, "TM") != 0)
		fatal(QUOTES("%s")" is not a compatible file format", p_path);

	if (!fread64_little_endian(p_vm->ip, file))
		fatal(QUOTES("%s")" is missing an entry point", p_path);

	if (!fread64_little_endian(&p_vm->data_segment_size, file))
		fatal(QUOTES("%s")" is missing the data segment size", p_path);

	if (!fread64_little_endian(&p_vm->program_size, file))
		fatal(QUOTES("%s")" is missing the program size", p_path);

	assert(INST_SIZE == 10);

	SMEMALLOC(p_vm->data_segment, p_vm->data_segment_size);
	SMEMALLOC(p_vm->program,      p_vm->program_size);

	for (size_t i = 0; i < p_vm->data_segment_size; ++ i) {
		tmp = fread(&p_vm->data_segment[i], 1, 1, file);
		if (tmp < 1)
			fatal(QUOTES("%s")" got an error while reading data segment", p_path);
	}

	for (size_t i = 0; i < p_vm->program_size; ++ i) {
		uint8_t bytes[INST_SIZE];

		tmp = fread(bytes, 1, sizeof(bytes), file);
		if (tmp < (int)sizeof(bytes))
			fatal(QUOTES("%s")" has an incomplete instruction", p_path);

		p_vm->program[i].opcode = bytes[0];
		p_vm->program[i].reg    = bytes[1];
		p_vm->program[i].data   = ((uint64_t)bytes[2] << 070) |
		                          ((uint64_t)bytes[3] << 060) |
		                          ((uint64_t)bytes[4] << 050) |
		                          ((uint64_t)bytes[5] << 040) |
		                          ((uint64_t)bytes[6] << 030) |
		                          ((uint64_t)bytes[7] << 020) |
		                          ((uint64_t)bytes[8] << 010) |
		                          ((uint64_t)bytes[9]);
	}

	fclose(file);
}

int vm_exec(vm_t *p_vm) {
	/* allocate the static memory */
	uint8_t static_memory[p_vm->data_segment_size + STACK_SIZE];
	p_vm->static_memory = static_memory;

	/* copy the data segment into the static memory */
	memcpy(p_vm->static_memory, p_vm->data_segment, p_vm->data_segment_size);

	p_vm->regs[REG_SB] = p_vm->data_segment_size;
	p_vm->regs[REG_SP] = p_vm->regs[REG_SB];

	for (p_vm->halt = false; !p_vm->halt; ++ *p_vm->ip) {
		if (*p_vm->ip >= p_vm->program_size)
			vm_panic(p_vm, ERR_INVALID_ACCESS);

		vm_exec_inst(p_vm, &p_vm->program[*p_vm->ip]);
	}

	fflush(stdout);
	fflush(stderr);

	SMEMFREE(p_vm->program);
	SMEMFREE(p_vm->data_segment);

	return p_vm->regs[REG_EX];
}

void vm_dump(vm_t *p_vm, FILE *p_stream) {
	fputs("[STACK]\n", p_stream);

	if (p_vm->regs[REG_SB] == *p_vm->sp)
		fputs("EMPTY", p_stream);
	else {
		for (size_t i = p_vm->regs[REG_SB]; i < *p_vm->sp; ++ i) {
			if (i != 0 && i % 12 == 0)
				fputc('\n', p_stream);

			fprintf(p_stream, "%02x ", p_vm->static_memory[i]);
		}
	}

	fputc('\n', p_stream);
}

void vm_panic(vm_t *p_vm, err_t p_err) {
	fprintclrf(stderr, "\x1bR"ERR_PREFIX"\x1bX");

	switch (p_err) {
	case ERR_STACK_OVERFLOW:      fputs("Stack overflow\n",  stderr); break;
	case ERR_STACK_UNDERFLOW:     fputs("Stack underflow\n", stderr); break;
	case ERR_UNKNOWN_INSTRUCTION: fprintf(stderr, "Unknown instruction 0x%02x\n",
	                                      p_vm->program[*p_vm->ip].opcode); break;
	case ERR_INVALID_ACCESS: fputs("Invalid access\n",   stderr); break;
	case ERR_DIV_BY_ZERO:    fputs("Division by zero\n", stderr); break;
	case ERR_WRITE_TO_READ_ONLY:
		fputs("Attempt to write into read-only memory\n", stderr);

		break;

	default: fputs(ERR_PREFIX"Unknown error\n", stderr);
	}

	fprintclrf(stderr, "  -> At instruction \x1bN0x%017lx\x1bX\n", (long)*p_vm->ip);

	/* debug
	fputs("\nDebug:\n", stderr);
	vm_dump(p_vm, stderr);
	*/

	exit(EXIT_FAILURE);
}

void vm_exec_inst(vm_t *p_vm, inst_t *p_inst) {
	switch (p_inst->opcode) {
	case OPCODE_NONE: break;
	case OPCODE_MOVE:
		*vm_access_reg(p_vm, p_inst->reg) = p_inst->data;

		break;

	case OPCODE_MOVE_R:
		*vm_access_reg(p_vm, p_inst->reg) = *vm_access_reg(p_vm, p_inst->data);

		break;

	case OPCODE_WRITE_64:
		vm_write64(p_vm, *vm_access_reg(p_vm, p_inst->reg), p_inst->data);

		break;

	case OPCODE_WRITE_R_64:
		vm_write64(p_vm, *vm_access_reg(p_vm, p_inst->reg), *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_WRITE_32:
		vm_write32(p_vm, *vm_access_reg(p_vm, p_inst->reg), p_inst->data);

		break;

	case OPCODE_WRITE_R_32:
		vm_write32(p_vm, *vm_access_reg(p_vm, p_inst->reg), *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_WRITE_16:
		vm_write16(p_vm, *vm_access_reg(p_vm, p_inst->reg), p_inst->data);

		break;

	case OPCODE_WRITE_R_16:
		vm_write16(p_vm, *vm_access_reg(p_vm, p_inst->reg), *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_WRITE_8:
		vm_write8(p_vm, *vm_access_reg(p_vm, p_inst->reg), p_inst->data);

		break;

	case OPCODE_WRITE_R_8:
		vm_write8(p_vm, *vm_access_reg(p_vm, p_inst->reg), *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_READ_64:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read64(p_vm, p_inst->data);

		break;

	case OPCODE_READ_R_64:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read64(p_vm, *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_READ_32:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read32(p_vm, p_inst->data);

		break;

	case OPCODE_READ_R_32:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read32(p_vm, *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_READ_16:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read16(p_vm, p_inst->data);

		break;

	case OPCODE_READ_R_16:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read16(p_vm, *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_READ_8:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read8(p_vm, p_inst->data);

		break;

	case OPCODE_READ_R_8:
		*vm_access_reg(p_vm, p_inst->reg) = vm_read8(p_vm, *vm_access_reg(p_vm, p_inst->data));

		break;

	case OPCODE_PUSH_64:
		if (*p_vm->sp + sizeof(uint64_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, p_inst->data);
		*p_vm->sp += sizeof(uint64_t);

		break;

	case OPCODE_PUSH_R_64:
		if (*p_vm->sp + sizeof(uint64_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, *vm_access_reg(p_vm, p_inst->reg));
		*p_vm->sp += sizeof(uint64_t);

		break;

	case OPCODE_PUSH_32:
		if (*p_vm->sp + sizeof(uint32_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write32(p_vm, *p_vm->sp, p_inst->data);
		*p_vm->sp += sizeof(uint32_t);

		break;

	case OPCODE_PUSH_R_32:
		if (*p_vm->sp + sizeof(uint32_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write32(p_vm, *p_vm->sp, *vm_access_reg(p_vm, p_inst->reg));
		*p_vm->sp += sizeof(uint32_t);

		break;

	case OPCODE_PUSH_16:
		if (*p_vm->sp + sizeof(uint16_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write16(p_vm, *p_vm->sp, p_inst->data);
		*p_vm->sp += sizeof(uint16_t);

		break;

	case OPCODE_PUSH_R_16:
		if (*p_vm->sp + sizeof(uint16_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write16(p_vm, *p_vm->sp, *vm_access_reg(p_vm, p_inst->reg));
		*p_vm->sp += sizeof(uint16_t);

		break;

	case OPCODE_PUSH_8:
		if (*p_vm->sp + sizeof(uint8_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write8(p_vm, *p_vm->sp, p_inst->data);
		*p_vm->sp += sizeof(uint8_t);

		break;

	case OPCODE_PUSH_R_8:
		if (*p_vm->sp + sizeof(uint8_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write8(p_vm, *p_vm->sp, *vm_access_reg(p_vm, p_inst->reg));
		*p_vm->sp += sizeof(uint8_t);

		break;

	case OPCODE_PUSH_A:
		if (*p_vm->sp + sizeof(word_t) * GEN_REGS_COUNT > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		for (size_t i = REG_1; i < GEN_REGS_COUNT - REG_1; ++ i, *p_vm->sp += sizeof(word_t))
			vm_write64(p_vm, *p_vm->sp, p_vm->regs[i]);

		break;

	case OPCODE_POP_64:
		/* *p_vm->sp is unsigned, not signed, so if sizeof(word_t) is greater than *p_vm->sp,
		   the subtract operation will result in negative values which will wrap around and
		   mess things up. This comparison prevents that */
		if (*p_vm->sp < sizeof(uint64_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint64_t);

		break;

	case OPCODE_POP_R_64:
		if (*p_vm->sp < sizeof(uint64_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint64_t);
		*vm_access_reg(p_vm, p_inst->reg) = vm_read64(p_vm, *p_vm->sp);

		break;

	case OPCODE_POP_32:
		if (*p_vm->sp < sizeof(uint32_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint32_t);

		break;

	case OPCODE_POP_R_32:
		if (*p_vm->sp < sizeof(uint32_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint32_t);
		*vm_access_reg(p_vm, p_inst->reg) = vm_read32(p_vm, *p_vm->sp);

		break;

	case OPCODE_POP_16:
		if (*p_vm->sp < sizeof(uint16_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint16_t);

		break;

	case OPCODE_POP_R_16:
		if (*p_vm->sp < sizeof(uint16_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint16_t);
		*vm_access_reg(p_vm, p_inst->reg) = vm_read16(p_vm, *p_vm->sp);

		break;

	case OPCODE_POP_8:
		if (*p_vm->sp < sizeof(uint8_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint8_t);

		break;

	case OPCODE_POP_R_8:
		if (*p_vm->sp < sizeof(uint8_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint8_t);
		*vm_access_reg(p_vm, p_inst->reg) = vm_read8(p_vm, *p_vm->sp);

		break;

	case OPCODE_POP_A:
		{ /* i stored the value in a variable to not repeat the same operation and to shorten
		     the line */
			size_t bytes_to_write = sizeof(word_t) * GEN_REGS_COUNT;
			if (*p_vm->sp < bytes_to_write)
				vm_panic(p_vm, ERR_STACK_UNDERFLOW);
		}

		for (size_t i = REG_1; i < GEN_REGS_COUNT - REG_1; ++ i) {
			*p_vm->sp -= sizeof(word_t);
			p_vm->regs[GEN_REGS_COUNT - i - 1] = vm_read64(p_vm, *p_vm->sp);
		}

		break;

	case OPCODE_INC: ++ *vm_access_reg(p_vm, p_inst->reg);              break;
	case OPCODE_ADD: *vm_access_reg(p_vm, p_inst->reg) += p_inst->data; break;
	case OPCODE_ADD_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			*r1 += *r2;
		}

		break;

	case OPCODE_DEC: -- *vm_access_reg(p_vm, p_inst->reg);              break;
	case OPCODE_SUB: *vm_access_reg(p_vm, p_inst->reg) -= p_inst->data; break;
	case OPCODE_SUB_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			*r1 -= *r2;
		}

		break;

	case OPCODE_MULT: *vm_access_reg(p_vm, p_inst->reg) *= p_inst->data; break;
	case OPCODE_MULT_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			*r1 *= *r2;
		}

		break;

	case OPCODE_DIV:
		{
			if (p_inst->data == 0)
				vm_panic(p_vm, ERR_DIV_BY_ZERO);

			word_t *r1 = vm_access_reg(p_vm, p_inst->reg);
			*r1 /= p_inst->data;

			 p_vm->regs[REG_AC] = *r1 % p_inst->data;
		}

		break;

	case OPCODE_DIV_R:
		{
			word_t *r1 = vm_access_reg(p_vm,  p_inst->reg);
			if (*r1 == 0)
				vm_panic(p_vm, ERR_DIV_BY_ZERO);

			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			*r1 /= *r2;

			 p_vm->regs[REG_AC] = *r1 % *r2;
		}

		break;

	case OPCODE_MOD: *vm_access_reg(p_vm, p_inst->reg) %= p_inst->data; break;
	case OPCODE_MOD_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			*r1 %= *r2;
		}

		break;

	case OPCODE_EQ:
		p_vm->regs[REG_CN] = *vm_access_reg(p_vm, p_inst->reg) == p_inst->data;

		break;

	case OPCODE_EQ_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			p_vm->regs[REG_CN] = *r1 == *r2;
		}

		break;

	case OPCODE_NEQ:
		p_vm->regs[REG_CN] = *vm_access_reg(p_vm, p_inst->reg) != p_inst->data;

		break;

	case OPCODE_NEQ_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			p_vm->regs[REG_CN] = *r1 != *r2;
		}

		break;

	case OPCODE_GT:
		p_vm->regs[REG_CN] = *vm_access_reg(p_vm, p_inst->reg) > p_inst->data;

		break;

	case OPCODE_GT_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			p_vm->regs[REG_CN] = *r1 > *r2;
		}

		break;

	case OPCODE_GE:
		p_vm->regs[REG_CN] = *vm_access_reg(p_vm, p_inst->reg) >= p_inst->data;

		break;

	case OPCODE_GE_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			p_vm->regs[REG_CN] = *r1 >= *r2;
		}

		break;

	case OPCODE_LT:
		p_vm->regs[REG_CN] = *vm_access_reg(p_vm, p_inst->reg) < p_inst->data;

		break;

	case OPCODE_LT_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			p_vm->regs[REG_CN] = *r1 < *r2;
		}

		break;

	case OPCODE_LE:
		p_vm->regs[REG_CN] = *vm_access_reg(p_vm, p_inst->reg) <= p_inst->data;

		break;

	case OPCODE_LE_R:
		{
			word_t *r1 = vm_access_reg(p_vm,        p_inst->reg);
			word_t *r2 = vm_access_reg(p_vm, (reg_t)p_inst->data);

			p_vm->regs[REG_CN] = *r1 <= *r2;
		}

		break;

	case OPCODE_JUMP:   *p_vm->ip = p_inst->data - 1;                      break;
	case OPCODE_JUMP_R: *p_vm->ip = *vm_access_reg(p_vm, p_inst->reg) - 1; break;

	case OPCODE_JUMP_T:
		if (p_vm->regs[REG_CN] == 1)
			*p_vm->ip = p_inst->data - 1;

		break;

	case OPCODE_JUMP_T_R:
		if (p_vm->regs[REG_CN] == 1)
			*p_vm->ip = *vm_access_reg(p_vm, p_inst->reg) - 1;

		break;

	case OPCODE_JUMP_F:
		if (p_vm->regs[REG_CN] == 0)
			*p_vm->ip = p_inst->data - 1;

		break;

	case OPCODE_JUMP_F_R:
		if (p_vm->regs[REG_CN] == 0)
			*p_vm->ip = *vm_access_reg(p_vm, p_inst->reg) - 1;

		break;

	case OPCODE_CALL:
		if (*p_vm->sp + sizeof(word_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, p_vm->regs[REG_IP] + 1);
		*p_vm->sp += sizeof(word_t);

		*p_vm->ip = p_inst->data - 1;

		break;

	case OPCODE_CALL_R:
		if (*p_vm->sp + sizeof(word_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, p_vm->regs[REG_IP] + 1);
		*p_vm->sp += sizeof(word_t);

		*p_vm->ip = *vm_access_reg(p_vm, p_inst->reg) - 1;

		break;

	case OPCODE_CALL_T:
		if (p_vm->regs[REG_CN] != 1)
			break;

		if (*p_vm->sp + sizeof(word_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, *p_vm->ip + 1);
		*p_vm->sp += sizeof(word_t);

		*p_vm->ip = p_inst->data - 1;

		break;

	case OPCODE_CALL_T_R:
		if (p_vm->regs[REG_CN] != 1)
			break;

		if (*p_vm->sp + sizeof(word_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, *p_vm->ip + 1);
		*p_vm->sp += sizeof(word_t);

		*p_vm->ip = *vm_access_reg(p_vm, p_inst->reg) - 1;

		break;

	case OPCODE_CALL_F:
		if (p_vm->regs[REG_CN] != 0)
			break;

		if (*p_vm->sp + sizeof(word_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, *p_vm->ip + 1);
		*p_vm->sp += sizeof(word_t);

		*p_vm->ip = p_inst->data - 1;

		break;

	case OPCODE_CALL_F_R:
		if (p_vm->regs[REG_CN] != 0)
			break;

		if (*p_vm->sp + sizeof(word_t) > STACK_SIZE)
			vm_panic(p_vm, ERR_STACK_OVERFLOW);

		vm_write64(p_vm, *p_vm->sp, *p_vm->ip + 1);
		*p_vm->sp += sizeof(word_t);

		*p_vm->ip = *vm_access_reg(p_vm, p_inst->reg) - 1;

		break;

	case OPCODE_RET:
		if (*p_vm->sp < sizeof(uint64_t))
			vm_panic(p_vm, ERR_STACK_UNDERFLOW);

		*p_vm->sp -= sizeof(uint64_t);
		*p_vm->ip = vm_read64(p_vm, *p_vm->sp) - 1;

		break;

	case OPCODE_HALT: p_vm->halt = true; break;

	case OPCODE_SYSCALL:
	case OPCODE_SYSCALL_R:
		{
			syscall_t syscall;
			if (p_inst->opcode == OPCODE_SYSCALL)
				syscall = (syscall_t)p_inst->data;
			else
				syscall = (syscall_t)*vm_access_reg(p_vm, p_inst->reg);

			switch (syscall) {
			case SYSCALL_WRITEF:
				{
					word_t addr   = p_vm->regs[REG_1];
					word_t size   = p_vm->regs[REG_2];
					word_t count  = p_vm->regs[REG_3];
					word_t stream = p_vm->regs[REG_4];

					if (addr >= STACK_SIZE || addr + size * count > STACK_SIZE)
						vm_panic(p_vm, ERR_INVALID_ACCESS);

					/* TODO: implement a file descriptor system and
					   make 'write' write into a file */
					fwrite(&p_vm->static_memory[addr], size, count, stream == 2? stderr : stdout);
					/* 0 = stdin */
				}

				break;

			case SYSCALL_FLUSH: fflush(p_vm->regs[REG_1] == 2? stderr : stdout); break;
			case SYSCALL_MEMSET:
				{
					word_t addr  = p_vm->regs[REG_1];
					word_t size  = p_vm->regs[REG_2];
					word_t value = p_vm->regs[REG_3];

					if (addr >= STACK_SIZE || addr + size > STACK_SIZE)
						vm_panic(p_vm, ERR_INVALID_ACCESS);

					memset(&p_vm->static_memory[addr], value, size);
				}

				break;

			case SYSCALL_MEMCOPY:
				{
					word_t dest = p_vm->regs[REG_1];
					word_t src  = p_vm->regs[REG_2];
					word_t size = p_vm->regs[REG_3];

					if (dest >= STACK_SIZE || dest + size > STACK_SIZE)
						vm_panic(p_vm, ERR_INVALID_ACCESS);
					else if (src >= STACK_SIZE || src + size > STACK_SIZE)
						vm_panic(p_vm, ERR_INVALID_ACCESS);

					memcpy(&p_vm->static_memory[dest], &p_vm->static_memory[src], size);
				}

				break;

			case SYSCALL_DEBUG: vm_dump(p_vm, stdout); break;
			}
		}

		break;

	default: vm_panic(p_vm, ERR_UNKNOWN_INSTRUCTION);
	}
}

word_t *vm_access_reg(vm_t *p_vm, reg_t p_reg) {
	if ((int)p_reg >= REGS_COUNT)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	return &p_vm->regs[p_reg];
}

uint8_t *vm_access_mem(vm_t *p_vm, word_t p_addr, char p_for) {
	(void)p_vm; (void)p_addr; (void)p_for;

	/* TODO: make a function to access either static or dynamic memory */

	switch (p_for) {
	case 'w':

	case 'r':

	default: assert(0 && "p_for is not 'w' nor 'r'");
	}
}

uint8_t vm_read8(vm_t *p_vm, word_t p_addr) {
	if (p_addr >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	return p_vm->static_memory[p_addr];
}

uint16_t vm_read16(vm_t *p_vm, word_t p_addr) {
	if (p_addr + 1 >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	return ((uint16_t)p_vm->static_memory[p_addr] << 010) |
	        (uint16_t)p_vm->static_memory[p_addr + 1];
}

uint32_t vm_read32(vm_t *p_vm, word_t p_addr) {
	if (p_addr + 3 >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	return ((uint32_t)p_vm->static_memory[p_addr]     << 030) |
	       ((uint32_t)p_vm->static_memory[p_addr + 1] << 020) |
	       ((uint32_t)p_vm->static_memory[p_addr + 2] << 010) |
	        (uint32_t)p_vm->static_memory[p_addr + 3];
}

uint64_t vm_read64(vm_t *p_vm, word_t p_addr) {
	if (p_addr + 7 >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	return ((uint64_t)p_vm->static_memory[p_addr]     << 070) |
	       ((uint64_t)p_vm->static_memory[p_addr + 1] << 060) |
	       ((uint64_t)p_vm->static_memory[p_addr + 2] << 050) |
	       ((uint64_t)p_vm->static_memory[p_addr + 3] << 040) |
	       ((uint64_t)p_vm->static_memory[p_addr + 4] << 030) |
	       ((uint64_t)p_vm->static_memory[p_addr + 5] << 020) |
	       ((uint64_t)p_vm->static_memory[p_addr + 6] << 010) |
	        (uint64_t)p_vm->static_memory[p_addr + 7];
}

void vm_write8(vm_t *p_vm, word_t p_addr, uint8_t p_data) {
	if (p_addr >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	p_vm->static_memory[p_addr] = p_data;
}

void vm_write16(vm_t *p_vm, word_t p_addr, uint16_t p_data) {
	if (p_addr + 1 >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	p_vm->static_memory[p_addr]     = (p_data & 0xFF00) >> 010;
	p_vm->static_memory[p_addr + 1] = (p_data & 0x00FF);
}

void vm_write32(vm_t *p_vm, word_t p_addr, uint32_t p_data) {
	if (p_addr + 3 >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	p_vm->static_memory[p_addr]     = (p_data & 0xFF000000) >> 030;
	p_vm->static_memory[p_addr + 1] = (p_data & 0x00FF0000) >> 020;
	p_vm->static_memory[p_addr + 2] = (p_data & 0x0000FF00) >> 010;
	p_vm->static_memory[p_addr + 3] = (p_data & 0x000000FF);
}

void vm_write64(vm_t *p_vm, word_t p_addr, uint64_t p_data) {
	if (p_addr + 3 >= STACK_SIZE)
		vm_panic(p_vm, ERR_INVALID_ACCESS);

	p_vm->static_memory[p_addr]     = (p_data & 0xFF00000000000000) >> 070;
	p_vm->static_memory[p_addr + 1] = (p_data & 0x00FF000000000000) >> 060;
	p_vm->static_memory[p_addr + 2] = (p_data & 0x0000FF0000000000) >> 050;
	p_vm->static_memory[p_addr + 3] = (p_data & 0x000000FF00000000) >> 040;
	p_vm->static_memory[p_addr + 4] = (p_data & 0x00000000FF000000) >> 030;
	p_vm->static_memory[p_addr + 5] = (p_data & 0x0000000000FF0000) >> 020;
	p_vm->static_memory[p_addr + 6] = (p_data & 0x000000000000FF00) >> 010;
	p_vm->static_memory[p_addr + 7] = (p_data & 0x00000000000000FF);
}
