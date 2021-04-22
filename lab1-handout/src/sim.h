/*
 * sim.h
 *
 *  Created on: 2021Äê4ÔÂ8ÈÕ
 *      Author: zyr
 */

#ifndef SIM_H_
#define SIM_H_

#define WORD_WIDTH 32
#define BYTE_WIDTH 8
#define WORD_TYPE uint32_t
#define BYTE_TYPE uint8_t
#define INSTR_TYPE WORD_TYPE
#define GET_BITS(word, index, width) ((((WORD_TYPE)word) >> index) & (((WORD_TYPE)~0) >> (WORD_WIDTH - width)))
#define SET_BITS(word, index, width, val)   \
{\
	word = (((~((((WORD_TYPE) ~0) >> (WORD_WIDTH - width)) << index)) & word) | (val << index));\
}

void process_special(INSTR_TYPE);
void process_regimm(INSTR_TYPE);
void process_i_branch(INSTR_TYPE);//I-type branch
void process_shift(INSTR_TYPE);
void process_move(INSTR_TYPE instr);
void process_complex_alu(INSTR_TYPE instr);
void process_r_alu(INSTR_TYPE);
void process_i_alu(INSTR_TYPE);
void process_lw(INSTR_TYPE);
void process_sw(INSTR_TYPE);
#endif /* SIM_H_ */
