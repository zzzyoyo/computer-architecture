#include <stdio.h>
#include "shell.h"
#include "sim.h"

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */

	NEXT_STATE = CURRENT_STATE;//value copy
	NEXT_STATE.PC += 4;
	printf("%d instructions\n", INSTRUCTION_COUNT);

	INSTR_TYPE instr = mem_read_32(CURRENT_STATE.PC);
	printf("0x%x\n",instr);
	uint32_t opcode = GET_BITS(instr, WORD_WIDTH - 6, 6);
	uint32_t op_high = GET_BITS(instr, WORD_WIDTH - 3, 3);
	uint32_t op_low = GET_BITS(instr, WORD_WIDTH - 6, 3);
	switch(op_high){
	case(0):{
		switch(op_low){
		case(0):{
			//SPECIAL
			process_special(instr);
			break;
		}
		case(1):{
			//REGIMM
			process_regimm(instr);
			break;
		}
		case(2):{
			//J
			uint32_t nextpc = CURRENT_STATE.PC + 4;
			SET_BITS(nextpc, 2, 26, GET_BITS(instr, 0, 26));
			NEXT_STATE.PC = nextpc;
			break;
		}
		case(3):{
			//JAL
			NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 8;
			uint32_t nextpc = CURRENT_STATE.PC + 4;
			SET_BITS(nextpc, 2, 26, GET_BITS(instr, 0, 26));
			NEXT_STATE.PC = nextpc;
			break;
		}
		case(4):
		case(5):
		case(6):
		case(7):{
			//BRANCH
			process_i_branch(instr);
			break;
		}
		default:{
			printf("process_instruction case 0: unknown instruction\n");
			break;
		}
		}
		break;
	}
	case(1):{
		//ALU I-type
		process_i_alu(instr);
		break;
	}
	case(4):{
		//load word
		process_lw(instr);
		break;
	}
	case(5):{
		//save word
		process_sw(instr);
		break;
	}
	default:{
		printf("process_instruction: unknown instruction\n");
		break;
	}
    }
}


void process_lw(INSTR_TYPE instr){
	uint32_t op_low = GET_BITS(instr, WORD_WIDTH - 6, 3);
	int offset = (((int)GET_BITS(instr, 0, 16)) << 16)>>16;
	uint32_t base = GET_BITS(instr, 21, 5);
	uint32_t rt = GET_BITS(instr, 16, 5);
	uint32_t load_word = mem_read_32(CURRENT_STATE.REGS[base] + offset);
	switch(op_low){
	case(0):{
		//LB
		NEXT_STATE.REGS[rt] = (((int)GET_BITS(load_word, 0, 8)) << 24)>>24;
		break;
	}
	case(1):{
		//LH
		NEXT_STATE.REGS[rt] = (((int)GET_BITS(load_word, 0, 8)) << 16)>>16;
		break;
	}
	case(3):
	case(4):
	case(5):{
		//LW
		//LBU
		//LHU
		NEXT_STATE.REGS[rt] = load_word;
		break;
	}
	default:{
		printf("process_lw: unknown instruction\n");
		break;
	}
	}
}



void process_sw(INSTR_TYPE instr){
	uint32_t op_low = GET_BITS(instr, WORD_WIDTH - 6, 3);
	int offset = (((int)GET_BITS(instr, 0, 16)) << 16)>>16;
	uint32_t base = GET_BITS(instr, 21, 5);
	uint32_t address = CURRENT_STATE.REGS[base] + offset;
	uint32_t rt = GET_BITS(instr, 16, 5);
	switch(op_low){
	case(0):{
		//SB
		mem_write_32(address, GET_BITS(CURRENT_STATE.REGS[rt], 0, 8));
		break;
	}
	case(1):{
		mem_write_32(address, GET_BITS(CURRENT_STATE.REGS[rt], 0, 16));
		//SH
		break;
	}
	case(3):{
		//SW
		mem_write_32(address, CURRENT_STATE.REGS[rt]);
		break;
	}
	default:{
		printf("process_sw: unknown instruction\n");
		break;
	}
	}
}

void process_i_alu(INSTR_TYPE instr){
	uint32_t func_low = GET_BITS(instr, 0, 3);
	uint32_t rs = GET_BITS(instr, 21, 5);
	uint32_t rt = GET_BITS(instr, 16, 5);
	uint32_t uimm = GET_BITS(instr, 0, 16);
	int imm = (((int)uimm)<<16)>>16;
	switch(func_low){
	case(0):
	case(1):{
		//ADDI
		//ADDIU
		NEXT_STATE.REGS[rt] = (int)CURRENT_STATE.REGS[rs] + imm;
		break;
	}
	case(2):{
		//SLTI
		NEXT_STATE.REGS[rt] = ((int)CURRENT_STATE.REGS[rs] < imm);
		break;
	}
	case(3):{
		//SLTIU
		NEXT_STATE.REGS[rt] = (CURRENT_STATE.REGS[rs] < uimm);
		break;
	}
	case(4):{
		//ANDI
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & uimm;
		break;
	}
	case(5):{
		//ORI
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | uimm;
		break;
	}
	case(6):{
		//XORI
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ uimm;
		break;
	}
	case(7):{
		//LUI
		NEXT_STATE.REGS[rt] = uimm << 16;
		break;
	}
	default:{
		printf("process_i_alu: unknown instruction\n");
		break;
	}
	}
}


void process_complex_alu(INSTR_TYPE instr) {
	//multiple div
	uint32_t func_low = GET_BITS(instr, 0, 3);
	uint32_t rs = GET_BITS(instr, 21, 5);
	uint32_t rt = GET_BITS(instr, 16, 5);
	switch (func_low) {
	case (0): {
		//MULT
		long long result = (int) CURRENT_STATE.REGS[rs]
				* (int) CURRENT_STATE.REGS[rt];
		NEXT_STATE.LO = GET_BITS(result, 0, 32);
		result >>= 32;
		NEXT_STATE.HI = GET_BITS(result, 0, 32);
		break;
	}
	case (1): {
		//MULTU
		uint64_t result = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
		NEXT_STATE.LO = GET_BITS(result, 0, 32);
		result >>= 32;
		NEXT_STATE.HI = GET_BITS(result, 0, 32);
		break;
	}
	case (2): {
		//DIV
		int reg_rs = (int) CURRENT_STATE.REGS[rs];
		int reg_rt = (int) CURRENT_STATE.REGS[rt];
		NEXT_STATE.LO = reg_rs / reg_rt;
		NEXT_STATE.HI = reg_rs % reg_rt;
		break;
	}
	case (3): {
		//DIVU
		uint32_t reg_rs = (int) CURRENT_STATE.REGS[rs];
		uint32_t reg_rt = (int) CURRENT_STATE.REGS[rt];
		NEXT_STATE.LO = reg_rs / reg_rt;
		NEXT_STATE.HI = reg_rs % reg_rt;
		break;
	}
	default: {
		printf("process_complex_alu: unknown instruction\n");
		break;
	}
	}
}

void process_special(INSTR_TYPE instr){
	uint32_t func_high = GET_BITS(instr, 3, 3);
	switch(func_high){
	case(0):{
		//shift
		process_shift(instr);
		break;
	}
	case(1):{
		uint32_t func_low = GET_BITS(instr, 0, 3);
		switch(func_low){
		case(0):{
			//JR
			uint32_t rs = GET_BITS(instr, 21, 5);
			NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
			break;
		}
		case(1):{
			//JALR
			uint32_t rs = GET_BITS(instr, 21, 5);
			uint32_t rd = GET_BITS(instr, 11, 5);
			NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
			NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;
			break;
		}
		case(4):{
			//SYSCALL
			printf("SYSCALL\n");
//			if(CURRENT_STATE.REGS[2] == 0x0a){
//				RUN_BIT = 0;
//			}
			RUN_BIT = FALSE;
			break;
		}
		default:{
			printf("process_special case 1: unknown instruction\n");
			break;
		}
		}
		break;
	}
	case(2):{
		//move
		process_move(instr);
		break;
	}
	case(3):{
		//multiple div
		process_complex_alu(instr);
		break;
	}
	case(4):{
		//ALU R-type
		process_r_alu(instr);
		break;
	}
	case(5):{
		//SLT
		uint32_t func_low = GET_BITS(instr, 0, 3);
		int compare;
		uint32_t rs = GET_BITS(instr, 21, 5);
		uint32_t rt = GET_BITS(instr, 16, 5);
		uint32_t rd = GET_BITS(instr, 11, 5);
		switch(func_low){
		case(2):{
			//SLT
			compare = (int)CURRENT_STATE.REGS[rs] < (int)CURRENT_STATE.REGS[rt];
			break;
		}
		case(3):{
			//SLTU
			compare = CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt];
			break;
		}
		default:{
			printf("process_special case 5: unknown special instruction\n");
			break;
		}
		}
		if(compare == TRUE){
			CURRENT_STATE.REGS[rd] = 1;
		}
		else{
			CURRENT_STATE.REGS[rd] = 0;
		}
		break;
	}
	default:{
		printf("process_special: unknown special instruction\n");
		break;
	}
	}
}

void process_r_alu(INSTR_TYPE instr){
	uint32_t func_low = GET_BITS(instr, 0, 3);
	uint32_t rs = GET_BITS(instr, 21, 5);
	uint32_t rt = GET_BITS(instr, 16, 5);
	uint32_t rd = GET_BITS(instr, 11, 5);
	switch(func_low){
	case(0):{
		//ADD
		NEXT_STATE.REGS[rd] = (int)CURRENT_STATE.REGS[rs] + (int)CURRENT_STATE.REGS[rt];
		break;
	}
	case(1):{
		//ADDU
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
		break;
	}
	case(2):{
		//sub
		NEXT_STATE.REGS[rd] = (int)CURRENT_STATE.REGS[rs] - (int)CURRENT_STATE.REGS[rt];
		break;
	}
	case(3):{
		//SUBU
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
		break;
	}
	case(4):{
		//AND
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
		break;
	}
	case(5):{
		//OR
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
		break;
	}
	case(6):{
		//XOR
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
		break;
	}
	case(7):{
		//NOR
		NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
		break;
	}
	default:{
		printf("process_r_alu: unknown special instruction\n");
		break;
	}
	}
}

void process_move(INSTR_TYPE instr){
	uint32_t func_low = GET_BITS(instr, 0, 3);
	uint32_t rs = GET_BITS(instr, 21, 5);
	uint32_t rd = GET_BITS(instr, 11, 5);
	switch(func_low){
	case(0):{
		//MFHI
		NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
		break;
	}
	case(1):{
		//MTHI
		NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
		break;
	}
	case(2):{
		//MFLO
		NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
		break;
	}
	case(3):{
		//MTLO
		NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
		break;
	}
	default:{
		printf("process_move: unknown move instruction\n");
		break;
	}
	}
}

void process_shift(INSTR_TYPE instr){
	uint32_t func_low = GET_BITS(instr, 0, 3);
	uint32_t sa = GET_BITS(instr, 6, 5);
	uint32_t rs = GET_BITS(instr, 21, 5);
	uint32_t rt = GET_BITS(instr, 16, 5);
	uint32_t rd = GET_BITS(instr, 11, 5);
	switch(func_low){
	case(0):{
		//SLL
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << sa;
		break;
	}
	case(1):{
		//SRL
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
		break;
	}
	case(2):{
		//SRA
		int reg_rt = (int)CURRENT_STATE.REGS[rt];
		NEXT_STATE.REGS[rd] = reg_rt  >> sa;
		break;
	}
	case(3):{
		//SLLV
		uint32_t sa = GET_BITS(CURRENT_STATE.REGS[rs], 0, 5);
		NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << sa;
		break;
	}
	case(4):{
		//SLLV
		uint32_t sa = GET_BITS(CURRENT_STATE.REGS[rs], 0, 5);
		int reg_rt = (int)CURRENT_STATE.REGS[rt];
		NEXT_STATE.REGS[rd] = reg_rt  >> sa;
		break;
	}
	default:{
		printf("process_shift: unknown shift instruction\n");
		break;
	}
	}
}


void process_regimm(INSTR_TYPE instr){
	uint32_t rt_high = GET_BITS(instr, 19, 2);
	uint32_t rt_low = GET_BITS(instr, 16, 2);
	int compare;
	uint32_t rs = GET_BITS(instr, 21, 5);
	switch(rt_high){
	case(0):{
		switch(rt_low){
		case(0):{
			//BLTZ
			compare = ((int)CURRENT_STATE.REGS[rs] < 0);
			break;
		}
		case(1):{
			//BGEZ
			compare = ((int)CURRENT_STATE.REGS[rs] >= 0);
		}
		default:{
			printf("process_regimm case 0: unknown branch instruction\n");
			break;
		}
		}
		break;
	}
	case(2):{
		switch(rt_low){
		case(0):{
			//BLTZAL
			compare = ((int)CURRENT_STATE.REGS[rs] < 0);
			NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
			break;
		}
		case(1):{
			//BGEZAL
			compare = ((int)CURRENT_STATE.REGS[rs] >= 0);
			NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
		}
		default:{
			printf("process_regimm case 2: unknown branch instruction\n");
			break;
		}
		}
		break;
	}
	default:{
		printf("process_regimm: unknown regimm instruction\n");
		break;
	}
	}
	int offset = (int)GET_BITS(instr, 0, 16);
	NEXT_STATE.PC = CURRENT_STATE.PC + 4 + ((offset << 16) >> 14);
}

void process_i_branch(INSTR_TYPE instr){
	uint32_t op_low = GET_BITS(instr, WORD_WIDTH - 6, 3);
	uint32_t rs = GET_BITS(instr, 21, 5);
	uint32_t rt = GET_BITS(instr, 16, 5);
	int branch;
	switch(op_low){
	case(4):{
		//BEQ
		branch = (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]);
		break;
	}
	case(5):{
		//BNE
		branch = (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]);
		break;
	}
	case(6):{
		//BLEZ
		branch = (CURRENT_STATE.REGS[rs] == 0) | (GET_BITS(CURRENT_STATE.REGS[rs], 31, 1) == 1);
		break;
	}
	case(7):{
		//BGTZ
		branch = (CURRENT_STATE.REGS[rs] > 0);
		break;
	}
	default:{
		printf("process_i_branch: unknown I-type branch\n");
	}
	}
	int offset = (int)GET_BITS(instr, 0, 16);
	offset = ((offset << 16) >> 14);
	NEXT_STATE.PC = CURRENT_STATE.PC + 4 + offset;
}
