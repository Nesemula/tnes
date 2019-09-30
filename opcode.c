static void reset(void);
static void fetch_opcode(void);

static address effective_addr;
static uint8_t tmp_data;

/* Auxiliary functions */
inline static void update_flags_ZN(uint8_t reg) {
	status_n = (reg & 0x80);
	status_z = !reg;
}

inline static uint8_t group_status_flags(void) {
	uint8_t p = 0x30;
	if (status_n) p |= 0x80;
	if (status_v) p |= 0x40;
	if (status_d) p |= 0x08;
	if (status_i) p |= 0x04;
	if (status_z) p |= 0x02;
	if (status_c) p |= 0x01;
	return p;
}

inline static void ungroup_status_flags(uint8_t p) {
	status_n = p & 0x80;
	status_v = p & 0x40;
	status_d = p & 0x08;
	status_i = p & 0x04;
	status_z = p & 0x02;
	status_c = p & 0x01;
}

static void terminate(void) {
	puts("terminate");
	force_display();
#undef getchar
	getchar();
	exit(1);
}

static void idle(void) {
	// in JSR, register S receives here the addrL just readed, and S only is correctly updated in cycle 6 (branch_any_page)
	puts("idle");
}

/****************************************************** Status setting *****************************************************/
static void set_flag_C(void) {
	puts("set_flag_C");
	status_c = true;
	fetch_opcode();
}

static void set_flag_D(void) {
	puts("set_flag_D");
	status_d = true;
	fetch_opcode();
}

static void set_flag_I(void) {
	puts("set_flag_I");
	status_i = true;
	fetch_opcode();
}

static void clear_flag_C(void) {
	puts("clear_flag_C");
	status_c = false;
	fetch_opcode();
}

static void clear_flag_D(void) {
	puts("clear_flag_D");
	status_d = false;
	fetch_opcode();
}

static void clear_flag_I(void) {
	puts("clear_flag_I");
	status_i = false;
	fetch_opcode();
}

static void clear_flag_V(void) {
	puts("clear_flag_V");
	status_v = false;
	fetch_opcode();
}

/******************************************************** Arithmetic *******************************************************/
static void add_with_carry(void) {
	puts("add_with_carry");
	uint16_t sum = A + tmp_data + status_c;
	status_c = (sum & 0x0100);
	// overflow: if both operands are positive, the result must be positive (same if both are negative)
	status_v = ~(A ^ tmp_data) & (A ^ sum) & 0x80;
	A = sum;
	update_flags_ZN(A);
	fetch_opcode();
}

static void subtract_with_carry(void) {
	puts("subtract_with_carry");
	tmp_data ^= 0xFF;
	add_with_carry();
}

static void increment_X(void) {
	puts("increment_X");
	X++;
	update_flags_ZN(X);
	fetch_opcode();
}

static void increment_Y(void) {
	puts("increment_Y");
	Y++;
	update_flags_ZN(Y);
	fetch_opcode();
}

static void increment_data(void) {
	puts("increment_data");
	tmp_data++;
}

static void decrement_X(void) {
	puts("decrement_X");
	X--;
	update_flags_ZN(X);
	fetch_opcode();
}

static void decrement_Y(void) {
	puts("decrement_Y");
	Y--;
	update_flags_ZN(Y);
	fetch_opcode();
}

static void decrement_data(void) {
	puts("decrement_data");
	tmp_data--;
}

/********************************************************* Logical *********************************************************/
static void bitwise_and(void) {
	puts("bitwise_and");
	A &= tmp_data;
	update_flags_ZN(A);
	fetch_opcode();
}

static void bitwise_or(void) {
	puts("bitwise_or");
	A |= tmp_data;
	update_flags_ZN(A);
	fetch_opcode();
}

static void bitwise_xor(void) {
	puts("bitwise_xor");
	A ^= tmp_data;
	update_flags_ZN(A);
	fetch_opcode();
}

/**************************************************** Register transfer ****************************************************/
static void transfer_AX(void) {
	puts("transfer_AX");
	X = A;
	update_flags_ZN(X);
	fetch_opcode();
}

static void transfer_XA(void) {
	puts("transfer_XA");
	A = X;
	update_flags_ZN(A);
	fetch_opcode();
}

static void transfer_AY(void) {
	puts("transfer_AY");
	Y = A;
	update_flags_ZN(Y);
	fetch_opcode();
}

static void transfer_YA(void) {
	puts("transfer_YA");
	A = Y;
	update_flags_ZN(A);
	fetch_opcode();
}

static void transfer_XS(void) {
	puts("transfer_XS");
	S = X;
	fetch_opcode();
}

static void transfer_SX(void) {
	puts("transfer_SX");
	X = S;
	update_flags_ZN(X);
	fetch_opcode();
}

/**************************************************** Bit manipulation *****************************************************/
static void shift_left_A(void) {
	puts("shift_left_A");
	status_c = (A & 0x80);
	A <<= 1;
	update_flags_ZN(A);
	fetch_opcode();
}

static void shift_left_data(void) {
	puts("shift_left_data");
	status_c = (tmp_data & 0x80);
	tmp_data <<= 1;
}

static void shift_right_A(void) {
	puts("shift_right_A");
	status_c = (A & 0x01);
	A >>= 1;
	update_flags_ZN(A);
	fetch_opcode();
}

static void shift_right_data(void) {
	puts("shift_right_data");
	status_c = (tmp_data & 0x01);
	tmp_data >>= 1;
}

static void rotate_left_A(void) {
	puts("rotate_left_A");
	bool carry = status_c;
	status_c = (A & 0x80);
	A <<= 1;
	if (carry)
		A |= 0x01;
	update_flags_ZN(A);
	fetch_opcode();
}

static void rotate_left_data(void) {
	puts("rotate_left_data");
	bool carry = status_c;
	status_c = (tmp_data & 0x80);
	tmp_data <<= 1;
	if (carry)
		tmp_data |= 0x01;
}

static void rotate_right_A(void) {
	puts("rotate_right_A");
	bool carry = status_c;
	status_c = (A & 0x01);
	A >>= 1;
	if (carry)
		A |= 0x80;
	update_flags_ZN(A);
	fetch_opcode();
}

static void rotate_right_data(void) {
	puts("rotate_right_data");
	bool carry = status_c;
	status_c = (tmp_data & 0x01);
	tmp_data >>= 1;
	if (carry)
		tmp_data |= 0x80;
}

/********************************************************* Memory **********************************************************/
static void put_data_into_A(void) {
	puts("put_data_into_A");
	A = tmp_data;
	update_flags_ZN(A);
	fetch_opcode();
}

static void put_data_into_X(void) {
	puts("put_data_into_X");
	X = tmp_data;
	update_flags_ZN(X);
	fetch_opcode();
}

static void put_data_into_Y(void) {
	puts("put_data_into_Y");
	Y = tmp_data;
	update_flags_ZN(Y);
	fetch_opcode();
}

static void store_A(void) {
	puts("store_A");
	write_memory(effective_address, A);
}

static void store_X(void) {
	puts("store_X");
	write_memory(effective_address, X);
}

static void store_Y(void) {
	puts("store_Y");
	write_memory(effective_address, Y);
}

static void store_data(void) {
	puts("store_data");
	write_memory(effective_address, tmp_data);
	update_flags_ZN(tmp_data);
}

/******************************************************* Comparison ********************************************************/
static void compare_A(void) {
	puts("compare_A");
	status_c = (A >= tmp_data);
	update_flags_ZN(A - tmp_data);
	fetch_opcode();
}

static void compare_X(void) {
	puts("compare_X");
	status_c = (X >= tmp_data);
	update_flags_ZN(X - tmp_data);
	fetch_opcode();
}

static void compare_Y(void) {
	puts("compare_Y");
	status_c = (Y >= tmp_data);
	update_flags_ZN(Y - tmp_data);
	fetch_opcode();
}

/******************************************************** Branching ********************************************************/
static void check_flag_Z_set(void) {
	puts("check_flag_Z_set");
	if (!status_z)
		fetch_opcode();
}

static void check_flag_N_set(void) {
	puts("check_flag_N_set");
	if (!status_n)
		fetch_opcode();
}

static void check_flag_C_set(void) {
	puts("check_flag_C_set");
	if (!status_c)
		fetch_opcode();
}

static void check_flag_V_set(void) {
	puts("check_flag_V_set");
	if (!status_v)
		fetch_opcode();
}

static void check_flag_Z_clr(void) {
	puts("check_flag_Z_clr");
	if (status_z)
		fetch_opcode();
}

static void check_flag_N_clr(void) {
	puts("check_flag_N_clr");
	if (status_n)
		fetch_opcode();
}

static void check_flag_C_clr(void) {
	puts("check_flag_C_clr");
	if (status_c)
		fetch_opcode();
}

static void check_flag_V_clr(void) {
	puts("check_flag_V_clr");
	if (status_v)
		fetch_opcode();
}

static void branch_same_page(void) {
	puts("branch_same_page");
	printf("  OFFSET %d\n", (signed char) tmp_data);
	effective_address = PC + (int8_t) tmp_data;
	if (PCH == effective_address_hi) {
		PC = effective_address;
		fetch_opcode();
	} else {
		PCL = effective_address_lo;
	}
}

static void branch_any_page(void) {
	puts("branch_any_page");
	PC = effective_address;
	fetch_opcode();
}

/***************************************************************************************************************************/
static void fetch_rubbish(void) {
	puts("fetch_rubbish");
	(void) PC;
}

static void fetch_param_data(void) {
	puts("fetch_param_data");
#if DBG
	tmp_data = dbg_data;
	printf("tmp_data ->\033[1;53m %02X \033[0m\n", tmp_data);
#else
	tmp_data = read_memory(PC);
#endif
	set_PC(PC + 1);
}

static void fetch_param_addr_zp(void) {
	puts("fetch_param_addr_zp");
	effective_address = read_memory(PC);
	set_PC(PC + 1);
}

static void fetch_param_addr_lo(void) {
	puts("fetch_param_addr_lo");
	effective_address_lo = read_memory(PC);
	set_PC(PC + 1);
}

static void fetch_param_addr_hi(void) {
	puts("fetch_param_addr_hi");
	effective_address_hi = read_memory(PC);
	set_PC(PC + 1);
}

static void fetch_any_data(void) {
	puts("fetch_any_data");
#if DBG
	tmp_data = dbg_data;
	printf("tmp_data ->\033[1;43m %02X \033[0m\n", tmp_data);
#else
	tmp_data = read_memory(effective_address);
#endif
}

static void add_X_to_addr_lo(void) {
	puts("add_X_to_addr_lo");
	effective_address_lo += X;
}

/***************************************************************************************************************************/
/* Read/write */
static void load_addr(void) {
	puts("load_addr");
	effective_address = read_memory(PC);
	set_PC(PC + 1);
}

static void load_addr_H(void) {
	puts("load_addr_H");
	effective_address_hi = read_memory(PC);
	set_PC(PC + 1);
}

/* Stack operations */
static void pull_P(void) {
	puts("pull_P");
	ungroup_status_flags(read_memory(++S | 0x100));
}

static void pull_PCL(void) {
	puts("pull_PCL");
	effective_address = read_memory(++S | 0x100);
}

static void pull_PCH(void) {
	// the stack is only updated here
	puts("pull_PCH");
	effective_address_hi = read_memory(0x100 | ++S);
	set_PC(effective_address);
}

static void push_PCH(void) {
	puts("push_PCH");
	write_memory(0x100 | S--, PCH);
}

static void push_PCL(void) {
	puts("push_PCL");
	write_memory(0x100 | S--, PCL);
}

static void push_P_with_B(void) {
	puts("push_P_with_B");
	unsigned char p = group_status_flags();
	p |= 0x14;
	write_memory(0x100 | S--, p);
}

static void push_A(void) { // S is only updated in next cycle
	puts("push_A");
	write_memory(0x100 | S--, A);
}

static void pull_A(void) {
	puts("pull_A");
	tmp_data = read_memory(0x100 |++S);
}

/* Memory access */
static void read_vector_L(void) {
	puts("read_vector_L");
	effective_address = read_memory(vector);
}

static void read_vector_H(void) {
	puts("read_vector_H");
	effective_address_hi = read_memory(vector + 1);
	//PC = effective_address;
	set_PC(effective_address);
}

static void read_addr(void) {
	puts("read_addr");
	tmp_data = read_memory(effective_address);
}

static void read_addr_H(void) {
	puts("read_addr_H");
	effective_address_hi = read_memory(effective_address + 1);
	effective_address_lo = tmp_data;
}

static void add_Y_to_addr(void) { // this in fact happens in the next cycle
	puts("add_Y_to_addr");
	effective_address += Y;
}

static void add_X_to_addr_and_read(void) {
	puts("add_X_to_addr_and_read");
	effective_address += X;
	tmp_data = read_memory(effective_address);
}

static void add_Y_to_addr_and_read(void) {
	puts("add_Y_to_addr_and_read");
	effective_address += Y;
	tmp_data = read_memory(effective_address);
}

static void add_X_to_addr_and_read_if_same_page(void) {
	puts("add_X_to_addr_and_read_if_same_page");
	if (effective_address_lo + X < 0x100) {
		effective_address += X;
		tmp_data = read_memory(effective_address);
		step++;
	}
}

static void add_Y_to_addr_and_read_if_same_page(void) {
	puts("add_Y_to_addr_and_read_if_same_page");
	if (effective_address_lo + Y < 0x100) {
		effective_address += Y;
		tmp_data = read_memory(effective_address);
		step++;
	}
}

opcode *RST_special    [] = { reset };
opcode *ERR_illegal    [] = { terminate };
opcode *BRK_stack      [] = { fetch_param_data, push_PCH, push_PCL, push_P_with_B, read_vector_L, read_vector_H, fetch_opcode };
opcode *RTI_stack      [] = { fetch_param_data, idle, pull_P, pull_PCL, pull_PCH, fetch_opcode };

/* IMPLIED ********************************************************************/
opcode *SEC_implied    [] = { fetch_rubbish, set_flag_C };
opcode *SED_implied    [] = { fetch_rubbish, set_flag_D };
opcode *SEI_implied    [] = { fetch_rubbish, set_flag_I };
opcode *CLC_implied    [] = { fetch_rubbish, clear_flag_C };
opcode *CLD_implied    [] = { fetch_rubbish, clear_flag_D };
opcode *CLI_implied    [] = { fetch_rubbish, clear_flag_I };
opcode *CLV_implied    [] = { fetch_rubbish, clear_flag_V };

opcode *INX_implied    [] = { fetch_rubbish, increment_X };
opcode *INY_implied    [] = { fetch_rubbish, increment_Y };
opcode *DEX_implied    [] = { fetch_rubbish, decrement_X };
opcode *DEY_implied    [] = { fetch_rubbish, decrement_Y };

opcode *TAX_implied    [] = { fetch_rubbish, transfer_AX };
opcode *TXA_implied    [] = { fetch_rubbish, transfer_XA };
opcode *TAY_implied    [] = { fetch_rubbish, transfer_AY };
opcode *TYA_implied    [] = { fetch_rubbish, transfer_YA };
opcode *TXS_implied    [] = { fetch_rubbish, transfer_XS };
opcode *TSX_implied    [] = { fetch_rubbish, transfer_SX };

opcode *NOP_implied    [] = { fetch_rubbish, fetch_opcode };


/* ACCUMULATOR ****************************************************************/
opcode *ASL_accumulator[] = { fetch_rubbish, shift_left_A };
opcode *LSR_accumulator[] = { fetch_rubbish, shift_right_A };
opcode *ROL_accumulator[] = { fetch_rubbish, rotate_left_A };
opcode *ROR_accumulator[] = { fetch_rubbish, rotate_right_A };


/* IMMMEDIATE *****************************************************************/
opcode *LDA_immediate  [] = { fetch_param_data, put_data_into_A };
opcode *LDX_immediate  [] = { fetch_param_data, put_data_into_X };
opcode *LDY_immediate  [] = { fetch_param_data, put_data_into_Y };

opcode *CMP_immediate  [] = { fetch_param_data, compare_A };
opcode *CPX_immediate  [] = { fetch_param_data, compare_X };
opcode *CPY_immediate  [] = { fetch_param_data, compare_Y };

opcode *ADC_immediate  [] = { fetch_param_data, add_with_carry };
opcode *SBC_immediate  [] = { fetch_param_data, subtract_with_carry };

opcode *AND_immediate  [] = { fetch_param_data, bitwise_and };
opcode *ORA_immediate  [] = { fetch_param_data, bitwise_or };
opcode *EOR_immediate  [] = { fetch_param_data, bitwise_xor };


/* RELATIVE *******************************************************************/
opcode *BEQ_relative   [] = { fetch_param_data, check_flag_Z_set, branch_same_page, branch_any_page };
opcode *BMI_relative   [] = { fetch_param_data, check_flag_N_set, branch_same_page, branch_any_page };
opcode *BCS_relative   [] = { fetch_param_data, check_flag_C_set, branch_same_page, branch_any_page };
opcode *BVS_relative   [] = { fetch_param_data, check_flag_V_set, branch_same_page, branch_any_page };
opcode *BNE_relative   [] = { fetch_param_data, check_flag_Z_clr, branch_same_page, branch_any_page };
opcode *BPL_relative   [] = { fetch_param_data, check_flag_N_clr, branch_same_page, branch_any_page };
opcode *BCC_relative   [] = { fetch_param_data, check_flag_C_clr, branch_same_page, branch_any_page };
opcode *BVC_relative   [] = { fetch_param_data, check_flag_V_clr, branch_same_page, branch_any_page };


/* ZERO PAGE ******************************************************************/
opcode *STA_zeropage   [] = { fetch_param_addr_zp, store_A, fetch_opcode };
opcode *STX_zeropage   [] = { fetch_param_addr_zp, store_X, fetch_opcode };
opcode *STY_zeropage   [] = { fetch_param_addr_zp, store_Y, fetch_opcode };

opcode *LDA_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, put_data_into_A };
opcode *LDX_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, put_data_into_X };
opcode *LDY_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, put_data_into_Y };

opcode *AND_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, bitwise_and };
opcode *ORA_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, bitwise_or };
opcode *EOR_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, bitwise_xor };

opcode *ADC_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, add_with_carry };

opcode *INC_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, increment_data,    store_data, fetch_opcode };
opcode *DEC_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, decrement_data,    store_data, fetch_opcode };

opcode *ASL_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, shift_left_data,   store_data, fetch_opcode };
opcode *LSR_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, shift_right_data,  store_data, fetch_opcode };
opcode *ROL_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, rotate_left_data,  store_data, fetch_opcode };
opcode *ROR_zeropage   [] = { fetch_param_addr_zp, fetch_any_data, rotate_right_data, store_data, fetch_opcode };


/* ABSOLUTE *******************************************************************/
opcode *STA_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, store_A, fetch_opcode };
opcode *STX_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, store_X, fetch_opcode };

opcode *LDA_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, fetch_any_data, put_data_into_A };
opcode *LDX_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, fetch_any_data, put_data_into_X };

opcode *CPX_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, fetch_any_data, compare_X };

opcode *JMP_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, branch_any_page };

opcode *JSR_absolute   [] = { load_addr, idle, push_PCH, push_PCL, load_addr_H, branch_any_page };


/* ZERO PAGE X ****************************************************************/
opcode *STA_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, store_A, fetch_opcode };

opcode *LDA_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, fetch_any_data, put_data_into_A };
opcode *LDY_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, fetch_any_data, put_data_into_Y };

opcode *AND_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, fetch_any_data, bitwise_and };

opcode *DEC_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, fetch_any_data, decrement_data,    store_data, fetch_opcode };

opcode *ROR_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, fetch_any_data, rotate_right_data, store_data, fetch_opcode };


/* ABSOLUTE X */
opcode *LDA_absoluteX  [] = { load_addr, load_addr_H, add_X_to_addr_and_read_if_same_page, add_X_to_addr_and_read, put_data_into_A };

/* INDIRECT Y */
opcode *STA_indirectY  [] = { load_addr, read_addr, read_addr_H, add_Y_to_addr, store_A, fetch_opcode };
opcode *LDA_indirectY  [] = { load_addr, read_addr, read_addr_H, add_Y_to_addr_and_read_if_same_page, add_Y_to_addr_and_read, put_data_into_A };

/* INDIRECT */
opcode *JMP_indirect   [] = { load_addr, load_addr_H, read_addr, read_addr_H, branch_any_page };

/* STACK */
opcode *RTS_stack      [] = { fetch_param_data, idle, pull_PCL, pull_PCH, fetch_param_data, fetch_opcode };
opcode *PHA_stack      [] = { fetch_rubbish, push_A, fetch_opcode };
opcode *PLA_stack      [] = { fetch_rubbish, idle, pull_A, put_data_into_A };

opcode **oop[] = { ERR_illegal, SEI_implied };

