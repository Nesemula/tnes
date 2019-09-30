#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "debug.h"

#define PC  program_counter.full
#define PCH program_counter.part.high
#define PCL program_counter.part.low

#define effective_address    effective_addr.full
#define effective_address_hi effective_addr.part.high
#define effective_address_lo effective_addr.part.low

#define INDIRECT  indirect_addr.full
#define INDIRECTH indirect_addr.part.high
#define INDIRECTL indirect_addr.part.low

#define   NMI_VECTOR 0xFFFA
#define RESET_VECTOR 0xFFFC
#define   IRQ_VECTOR 0xFFFE

typedef void opcode(void);

typedef union {
	struct {
		uint8_t low;
		uint8_t high;
	} part;
	uint16_t full;
} address;

uint8_t A = 0x00;
uint8_t X = 0x00;
uint8_t Y = 0x00;
uint8_t S = 0x00;
address program_counter;

bool status_n = false;
bool status_v = false;
bool status_d = false;
bool status_i = false;
bool status_z = false;
bool status_c = false;

int DMA_occured = 0;
int DMA_address = 0;
int NMI_occured = 0;
int step = 0;
uint16_t next_PC = 0x0000;
uint16_t vector = RESET_VECTOR;

void set_PC(uint16_t new_PC) {
	next_PC = new_PC;
}
void update_PC(void) {
	PC = next_PC;
}

// DEBUG
int dbg = 0;
uint8_t dbg_data;

#include "opcode.c"

opcode **current = RST_special;

static void reset(void) {
	unsigned char op = 0x00;
	printf("\nreset %02X \033[1;33m %s \033[0m %s\n", op, mnemonic[op], addressing[op]);
	read_memory(PC);
	set_PC(PC + 1);
	step = 0;
	current = BRK_stack;
}

static void fetch_opcode(void) {
#if DBG
	if (!dbg++) {
		A = 0xaa;
		X = 0x0f;
		Y = 0x01;
		S = 0x00;
		ungroup_status_flags(0x81);
	}
	unsigned char op = 0x35;
	dbg_data = 0x86;
#else
	if (NMI_occured) {
		NMI_occured = 0;
		printf("interrrupt ->\033[1;33m NMI \033[0m\n");
		vector = NMI_VECTOR;
		//op = 0x00;
		current = BRK_stack;
		step = 0;
		return;
	}
	unsigned char op = read_memory(PC);
#endif
	set_PC(PC + 1);
	printf("\nfetch %02X \033[1;33m %s \033[0m %s\n", op, mnemonic[op], addressing[op]);
	step = 0;
	switch (op) {
		case 0x05: current = ORA_zeropage;    break;
		case 0x06: current = ASL_zeropage;    break;
		case 0x09: current = ORA_immediate;   break;
		case 0x0A: current = ASL_accumulator; break;
		case 0x10: current = BPL_relative;    break;
		case 0x18: current = CLC_implied;     break;
		case 0x20: current = JSR_absolute;    break;
		case 0x25: current = AND_zeropage;    break;
		case 0x26: current = ROL_zeropage;    break;
		case 0x29: current = AND_immediate;   break;
		case 0x2A: current = ROL_accumulator; break;
		case 0x30: current = BMI_relative;    break;
		case 0x35: current = AND_zeropageX;   break;
		case 0x38: current = SEC_implied;     break;
		case 0x45: current = EOR_zeropage;    break;
		case 0x46: current = LSR_zeropage;    break;
		case 0x48: current = PHA_stack;       break;
		case 0x49: current = EOR_immediate;   break;
		case 0x4A: current = LSR_accumulator; break;
		case 0x4C: current = JMP_absolute;    break;
		case 0x50: current = BVC_relative;    break;
		case 0x58: current = CLI_implied;     break;
		case 0x60: current = RTS_stack;       break;
		case 0x65: current = ADC_zeropage;    break;
		case 0x66: current = ROR_zeropage;    break;
		case 0x68: current = PLA_stack;       break;
		case 0x69: current = ADC_immediate;   break;
		case 0x6A: current = ROR_accumulator; break;
		case 0x6C: current = JMP_indirect;    break;
		case 0x70: current = BVS_relative;    break;
		case 0x76: current = ROR_zeropageX;   break;
		case 0x78: current = SEI_implied;     break;
		case 0x84: current = STY_zeropage;    break;
		case 0x85: current = STA_zeropage;    break;
		case 0x86: current = STX_zeropage;    break;
		case 0x88: current = DEY_implied;     break;
		case 0x8A: current = TXA_implied;     break;
		case 0x8D: current = STA_absolute;    break;
		case 0x8E: current = STX_absolute;    break;
		case 0x90: current = BCC_relative;    break;
		case 0x91: current = STA_indirectY;   break;
		case 0x95: current = STA_zeropageX;   break;
		case 0x98: current = TYA_implied;     break;
		case 0x9A: current = TXS_implied;     break;
		case 0xA0: current = LDY_immediate;   break;
		case 0xA2: current = LDX_immediate;   break;
		case 0xA4: current = LDY_zeropage;    break;
		case 0xA5: current = LDA_zeropage;    break;
		case 0xA6: current = LDX_zeropage;    break;
		case 0xA8: current = TAY_implied;     break;
		case 0xA9: current = LDA_immediate;   break;
		case 0xAA: current = TAX_implied;     break;
		case 0xAD: current = LDA_absolute;    break;
		case 0xAE: current = LDX_absolute;    break;
		case 0xB0: current = BCS_relative;    break;
		case 0xB1: current = LDA_indirectY;   break;
		case 0xB4: current = LDY_zeropageX;   break;
		case 0xB5: current = LDA_zeropageX;   break;
		case 0xB8: current = CLV_implied;     break;
		case 0xBA: current = TSX_implied;     break;
		case 0xBD: current = LDA_absoluteX;   break;
		case 0xC0: current = CPY_immediate;   break;
		case 0xC6: current = DEC_zeropage;    break;
		case 0xC8: current = INY_implied;     break;
		case 0xC9: current = CMP_immediate;   break;
		case 0xCA: current = DEX_implied;     break;
		case 0xD0: current = BNE_relative;    break;
		case 0xD6: current = DEC_zeropageX;   break;
		case 0xD8: current = CLD_implied;     break;
		case 0xE0: current = CPX_immediate;   break;
		case 0xE6: current = INC_zeropage;    break;
		case 0xE8: current = INX_implied;     break;
		case 0xE9: current = SBC_immediate;   break;
		case 0xEC: current = CPX_absolute;    break;
		case 0xEA: current = NOP_implied;     break;
		case 0xF0: current = BEQ_relative;    break;
		case 0xF8: current = SED_implied;     break;
		default  : current = ERR_illegal;     break;
	}
}

void cpu_dma(unsigned char base_address) {
	DMA_occured = 1;
	DMA_address = base_address;
}

void cpu_interrupt(void) {
	NMI_occured = 1;
//opcode *x = fetch_opcode;
//ERR_illegal[0] = x;
}

unsigned long long counter = 0;
inline void cpu_exec(void) {
	update_PC();
	//if (DMA_occured) fetch_opcode();
	//else
	current[step++]();
	uint8_t P = group_status_flags();
	printf(">> A %02X, X %02X, Y %02X, S %02X, P %02X, PC %04X, %c%c%c%c%c%c%c%c #%llu\n", A, X, Y, S, P, PC,
		P & 0x80 ? 'n' : '.', P & 0x40 ? 'v' : '.', P & 0x20 ? 'x' : '.', P & 0x10 ? 'b' : '.',
			P & 0x08 ? 'd' : '.', P & 0x04 ? 'i' : '.', P & 0x02 ? 'z' : '.', P & 0x01 ? 'c' : '.', ++counter);
}

