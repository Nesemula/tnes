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

#define   NMI_VECTOR 0xFFFA
#define RESET_VECTOR 0xFFFC
#define   IRQ_VECTOR 0xFFFE

typedef void opcode(void);
typedef opcode * instruction;
void (*next_op)(void);

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

struct {
	bool n;
	bool v;
	bool b;
	bool d;
	bool i;
	bool z;
	bool c;
} flag;
#define flag_n flag.n
#define flag_v flag.v
#define flag_b flag.b
#define flag_d flag.d
#define flag_i flag.i
#define flag_z flag.z
#define flag_c flag.c

uint_fast8_t step = 0;

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
uint8_t IR;
unsigned long long counter = 0;

#include "opcode.c"

instruction const *current = RST_special;

static void next(void) {
	(*next_op)();
}

static void fetch_opcode(void) {
#if DBG
	unsigned char op = 0x24;
	if (!dbg++) {
		A = 0xaa;
		X = 0xff;
		Y = 0xff;
		S = 0x00;
		ungroup_status_flags(0x81);
	}
	else  op = 0x01;
	dbg_data = 0x11;
#else
	uint8_t op = read_memory(PC);
#endif
	IR = op;
	set_PC(PC + 1);
	printf("\nfetch %02X \033[1;33m %s \033[0m %s\n", op, mnemonic[op], addressing[op]);
	step = 0;
	switch (op) {
		case 0x01: current = ORA_indirectX;   break;
		case 0x05: current = ORA_zeropage;    break;
		case 0x06: current = ASL_zeropage;    break;
		case 0x08: current = PHP_stack;       break;
		case 0x09: current = ORA_immediate;   break;
		case 0x0A: current = ASL_accumulator; break;
		case 0x0D: current = ORA_absolute;    break;
		case 0x0E: current = ASL_absolute;    break;
		case 0x10: current = BPL_relative;    break;
		case 0x11: current = ORA_indirectY;   break;
		case 0x15: current = ORA_zeropageX;   break;
		case 0x16: current = ASL_zeropageX;   break;
		case 0x18: current = CLC_implied;     break;
		case 0x19: current = ORA_absoluteY;   break;
		case 0x1D: current = ORA_absoluteX;   break;
		case 0x1E: current = ASL_absoluteX;   break;
		case 0x20: current = JSR_absolute;    break;
		case 0x21: current = AND_indirectX;   break;
		case 0x24: current = BIT_zeropage;    break;
		case 0x25: current = AND_zeropage;    break;
		case 0x26: current = ROL_zeropage;    break;
		case 0x28: current = PLP_stack;       break;
		case 0x29: current = AND_immediate;   break;
		case 0x2A: current = ROL_accumulator; break;
		case 0x2C: current = BIT_absolute;    break;
		case 0x2D: current = AND_absolute;    break;
		case 0x2E: current = ROL_absolute;    break;
		case 0x30: current = BMI_relative;    break;
		case 0x31: current = AND_indirectY;   break;
		case 0x35: current = AND_zeropageX;   break;
		case 0x36: current = ROL_zeropageX;   break;
		case 0x38: current = SEC_implied;     break;
		case 0x39: current = AND_absoluteY;   break;
		case 0x3D: current = AND_absoluteX;   break;
		case 0x3E: current = ROL_absoluteX;   break;
		case 0x40: current = RTI_stack;       break;
		case 0x41: current = EOR_indirectX;   break;
		case 0x45: current = EOR_zeropage;    break;
		case 0x46: current = LSR_zeropage;    break;
		case 0x48: current = PHA_stack;       break;
		case 0x49: current = EOR_immediate;   break;
		case 0x4A: current = LSR_accumulator; break;
		case 0x4C: current = JMP_absolute;    break;
		case 0x4D: current = EOR_absolute;    break;
		case 0x4E: current = LSR_absolute;    break;
		case 0x50: current = BVC_relative;    break;
		case 0x51: current = EOR_indirectY;   break;
		case 0x55: current = EOR_zeropageX;   break;
		case 0x56: current = LSR_zeropageX;   break;
		case 0x58: current = CLI_implied;     break;
		case 0x59: current = EOR_absoluteY;   break;
		case 0x5D: current = EOR_absoluteX;   break;
		case 0x5E: current = LSR_absoluteX;   break;
		case 0x60: current = RTS_stack;       break;
		case 0x61: current = ADC_indirectX;   break;
		case 0x65: current = ADC_zeropage;    break;
		case 0x66: current = ROR_zeropage;    break;
		case 0x68: current = PLA_stack;       break;
		case 0x69: current = ADC_immediate;   break;
		case 0x6A: current = ROR_accumulator; break;
		case 0x6C: current = JMP_indirect;    break;
		case 0x6D: current = ADC_absolute;    break;
		case 0x6E: current = ROR_absolute;    break;
		case 0x70: current = BVS_relative;    break;
		case 0x71: current = ADC_indirectY;   break;
		case 0x75: current = ADC_zeropageX;   break;
		case 0x76: current = ROR_zeropageX;   break;
		case 0x78: current = SEI_implied;     break;
		case 0x79: current = ADC_absoluteY;   break;
		case 0x7D: current = ADC_absoluteX;   break;
		case 0x7E: current = ROR_absoluteX;   break;
		case 0x81: current = STA_indirectX;   break;
		case 0x84: current = STY_zeropage;    break;
		case 0x85: current = STA_zeropage;    break;
		case 0x86: current = STX_zeropage;    break;
		case 0x88: current = DEY_implied;     break;
		case 0x8A: current = TXA_implied;     break;
		case 0x8C: current = STY_absolute;    break;
		case 0x8D: current = STA_absolute;    break;
		case 0x8E: current = STX_absolute;    break;
		case 0x90: current = BCC_relative;    break;
		case 0x91: current = STA_indirectY;   break;
		case 0x94: current = STY_zeropageX;   break;
		case 0x95: current = STA_zeropageX;   break;
		case 0x96: current = STX_zeropageY;   break;
		case 0x98: current = TYA_implied;     break;
		case 0x99: current = STA_absoluteY;   break;
		case 0x9A: current = TXS_implied;     break;
		case 0x9D: current = STA_absoluteX;   break;
		case 0xA0: current = LDY_immediate;   break;
		case 0xA1: current = LDA_indirectX;   break;
		case 0xA2: current = LDX_immediate;   break;
		case 0xA4: current = LDY_zeropage;    break;
		case 0xA5: current = LDA_zeropage;    break;
		case 0xA6: current = LDX_zeropage;    break;
		case 0xA8: current = TAY_implied;     break;
		case 0xA9: current = LDA_immediate;   break;
		case 0xAA: current = TAX_implied;     break;
		case 0xAC: current = LDY_absolute;    break;
		case 0xAD: current = LDA_absolute;    break;
		case 0xAE: current = LDX_absolute;    break;
		case 0xB0: current = BCS_relative;    break;
		case 0xB1: current = LDA_indirectY;   break;
		case 0xB4: current = LDY_zeropageX;   break;
		case 0xB5: current = LDA_zeropageX;   break;
		case 0xB6: current = LDX_zeropageY;   break;
		case 0xB8: current = CLV_implied;     break;
		case 0xB9: current = LDA_absoluteY;   break;
		case 0xBA: current = TSX_implied;     break;
		case 0xBC: current = LDY_absoluteX;   break;
		case 0xBD: current = LDA_absoluteX;   break;
		case 0xBE: current = LDX_absoluteY;   break;
		case 0xC0: current = CPY_immediate;   break;
		case 0xC1: current = CMP_indirectX;   break;
		case 0xC4: current = CPY_zeropage;    break;
		case 0xC5: current = CMP_zeropage;    break;
		case 0xC6: current = DEC_zeropage;    break;
		case 0xC8: current = INY_implied;     break;
		case 0xC9: current = CMP_immediate;   break;
		case 0xCA: current = DEX_implied;     break;
		case 0xCC: current = CPY_absolute;    break;
		case 0xCD: current = CMP_absolute;    break;
		case 0xCE: current = DEC_absolute;    break;
		case 0xD0: current = BNE_relative;    break;
		case 0xD1: current = CMP_indirectY;   break;
		case 0xD5: current = CMP_zeropageX;   break;
		case 0xD6: current = DEC_zeropageX;   break;
		case 0xD8: current = CLD_implied;     break;
		case 0xD9: current = CMP_absoluteY;   break;
		case 0xDD: current = CMP_absoluteX;   break;
		case 0xDE: current = DEC_absoluteX;   break;
		case 0xE0: current = CPX_immediate;   break;
		case 0xE1: current = SBC_indirectX;   break;
		case 0xE4: current = CPX_zeropage;    break;
		case 0xE5: current = SBC_zeropage;    break;
		case 0xE6: current = INC_zeropage;    break;
		case 0xE8: current = INX_implied;     break;
		case 0xE9: current = SBC_immediate;   break;
		case 0xEA: current = NOP_implied;     break;
		case 0xEC: current = CPX_absolute;    break;
		case 0xED: current = SBC_absolute;    break;
		case 0xEE: current = INC_absolute;    break;
		case 0xF0: current = BEQ_relative;    break;
		case 0xF1: current = SBC_indirectY;   break;
		case 0xF5: current = SBC_zeropageX;   break;
		case 0xF6: current = INC_zeropageX;   break;
		case 0xF8: current = SED_implied;     break;
		case 0xF9: current = SBC_absoluteY;   break;
		case 0xFD: current = SBC_absoluteX;   break;
		case 0xFE: current = INC_absoluteX;   break;
		default  : current = ERR_illegal;     break;
	}
}

static void wait_for_mem(void) {
	puts("wait_for_mem");
	if (memory_auto_transfer())
		step--;
	else {
		//getchar();
		next();
	}
}

static void reset(void) {
	unsigned char op = 0x00;
	printf("\nreset %02X \033[1;33m %s \033[0m %s\n", op, mnemonic[op], addressing[op]);
	read_memory(PC);
	set_PC(PC + 1);
	flag_b = false;
	step = 0;
	current = BRK_stack;
	next_op = &fetch_opcode;
}

static void dma_setup(void) {
	printf("\ntransfer ->\033[1;33m DMA \033[0m\n");
	current = DMA_special;
	step = 0;
	next_op = &fetch_opcode;
}

static void nmi_setup(void) {
	printf("\ninterrupt ->\033[1;33m NMI \033[0m\n");
	vector = NMI_VECTOR;
	current = BRK_stack;
	step = 0;
	next_op = &fetch_opcode;
}

void cpu_hang(void) {
	next_op = &dma_setup;
}

void cpu_interrupt(void) {
	next_op = &nmi_setup;
	flag_b = false;
}

void cpu_exec(void) {
	update_PC();
	current[step++]();
#ifdef DBGOUT
	uint8_t P = group_status_flags();
	printf(">> A %02X, X %02X, Y %02X, S %02X, P %02X, PC %04X, %c%c%c%c%c%c%c%c #%llu\n", A, X, Y, S, P, PC,
		P & 0x80 ? 'n' : '.', P & 0x40 ? 'v' : '.', P & 0x20 ? 'x' : '.', P & 0x10 ? 'b' : '.',
			P & 0x08 ? 'd' : '.', P & 0x04 ? 'i' : '.', P & 0x02 ? 'z' : '.', P & 0x01 ? 'c' : '.', ++counter);
#else
	counter++;
#endif
}

