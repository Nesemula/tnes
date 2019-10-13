static void wait_for_mem(void);
static void reset(void);
static void next(void);

static address effective_addr;
static uint8_t tmp_data;

/* Auxiliary functions */
inline static void update_flags_ZN(uint8_t reg) {
	flag_n = (reg & 0x80);
	flag_z = !reg;
}

inline static uint8_t group_status_flags(void) {
	uint8_t p = 0x20;
	if (flag_n) p |= 0x80;
	if (flag_v) p |= 0x40;
	if (flag_b) p |= 0x10;
	if (flag_d) p |= 0x08;
	if (flag_i) p |= 0x04;
	if (flag_z) p |= 0x02;
	if (flag_c) p |= 0x01;
	return p;
}

inline static void ungroup_status_flags(uint8_t p) {
	flag_n = p & 0x80;
	flag_v = p & 0x40;
	// flag B is not updated
	flag_d = p & 0x08;
	flag_i = p & 0x04;
	flag_z = p & 0x02;
	flag_c = p & 0x01;
}

static void terminate(void) {
	puts("terminate");
	force_display();
	fprintf(stdout, "\nfetch %02X \033[1;33m %s \033[0m %s #%llu\n", IR, mnemonic[IR], addressing[IR], counter);
	int x;
	scanf("%d", &x);
	exit(1);
}

/****************************************************** Status setting *****************************************************/
static void set_flag_C(void) {
	puts("set_flag_C");
	flag_c = true;
	next();
}

static void set_flag_D(void) {
	puts("set_flag_D");
	flag_d = true;
	next();
}

static void set_flag_I(void) {
	puts("set_flag_I");
	flag_i = true;
	next();
}

static void clear_flag_C(void) {
	puts("clear_flag_C");
	flag_c = false;
	next();
}

static void clear_flag_D(void) {
	puts("clear_flag_D");
	flag_d = false;
	next();
}

static void clear_flag_I(void) {
	puts("clear_flag_I");
	flag_i = false;
	next();
}

static void clear_flag_V(void) {
	puts("clear_flag_V");
	flag_v = false;
	next();
}

/******************************************************** Arithmetic *******************************************************/
static void add_with_carry(void) {
	puts("add_with_carry");
	uint16_t sum = A + tmp_data + flag_c;
	flag_c = (sum & 0x0100);
	// overflow: if both operands are positive, the result must be positive (same if both are negative)
	flag_v = ~(A ^ tmp_data) & (A ^ sum) & 0x80;
	A = sum;
	update_flags_ZN(A);
	next();
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
	next();
}

static void increment_Y(void) {
	puts("increment_Y");
	Y++;
	update_flags_ZN(Y);
	next();
}

static void increment_data(void) {
	puts("increment_data");
	tmp_data++;
}

static void decrement_X(void) {
	puts("decrement_X");
	X--;
	update_flags_ZN(X);
	next();
}

static void decrement_Y(void) {
	puts("decrement_Y");
	Y--;
	update_flags_ZN(Y);
	next();
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
	next();
}

static void bitwise_or(void) {
	puts("bitwise_or");
	A |= tmp_data;
	update_flags_ZN(A);
	next();
}

static void bitwise_xor(void) {
	puts("bitwise_xor");
	A ^= tmp_data;
	update_flags_ZN(A);
	next();
}

/**************************************************** Register transfer ****************************************************/
static void transfer_AX(void) {
	puts("transfer_AX");
	X = A;
	update_flags_ZN(X);
	next();
}

static void transfer_XA(void) {
	puts("transfer_XA");
	A = X;
	update_flags_ZN(A);
	next();
}

static void transfer_AY(void) {
	puts("transfer_AY");
	Y = A;
	update_flags_ZN(Y);
	next();
}

static void transfer_YA(void) {
	puts("transfer_YA");
	A = Y;
	update_flags_ZN(A);
	next();
}

static void transfer_XS(void) {
	puts("transfer_XS");
	S = X;
	next();
}

static void transfer_SX(void) {
	puts("transfer_SX");
	X = S;
	update_flags_ZN(X);
	next();
}

/**************************************************** Bit manipulation *****************************************************/
static void shift_left_A(void) {
	puts("shift_left_A");
	flag_c = (A & 0x80);
	A <<= 1;
	update_flags_ZN(A);
	next();
}

static void shift_left_data(void) {
	puts("shift_left_data");
	flag_c = (tmp_data & 0x80);
	tmp_data <<= 1;
}

static void shift_right_A(void) {
	puts("shift_right_A");
	flag_c = (A & 0x01);
	A >>= 1;
	update_flags_ZN(A);
	next();
}

static void shift_right_data(void) {
	puts("shift_right_data");
	flag_c = (tmp_data & 0x01);
	tmp_data >>= 1;
}

static void rotate_left_A(void) {
	puts("rotate_left_A");
	bool carry = flag_c;
	flag_c = (A & 0x80);
	A <<= 1;
	if (carry)
		A |= 0x01;
	update_flags_ZN(A);
	next();
}

static void rotate_left_data(void) {
	puts("rotate_left_data");
	bool carry = flag_c;
	flag_c = (tmp_data & 0x80);
	tmp_data <<= 1;
	if (carry)
		tmp_data |= 0x01;
}

static void rotate_right_A(void) {
	puts("rotate_right_A");
	bool carry = flag_c;
	flag_c = (A & 0x01);
	A >>= 1;
	if (carry)
		A |= 0x80;
	update_flags_ZN(A);
	next();
}

static void rotate_right_data(void) {
	puts("rotate_right_data");
	bool carry = flag_c;
	flag_c = (tmp_data & 0x01);
	tmp_data >>= 1;
	if (carry)
		tmp_data |= 0x80;
}

/********************************************************* Memory **********************************************************/
static void put_data_into_A(void) {
	puts("put_data_into_A");
	A = tmp_data;
	update_flags_ZN(A);
	next();
}

static void put_data_into_X(void) {
	puts("put_data_into_X");
	X = tmp_data;
	update_flags_ZN(X);
	next();
}

static void put_data_into_Y(void) {
	puts("put_data_into_Y");
	Y = tmp_data;
	update_flags_ZN(Y);
	next();
}

static void put_data_into_P(void) {
	puts("put_data_into_P");
	ungroup_status_flags(tmp_data);
	next();
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
	flag_c = (A >= tmp_data);
	update_flags_ZN(A - tmp_data);
	next();
}

static void compare_X(void) {
	puts("compare_X");
	flag_c = (X >= tmp_data);
	update_flags_ZN(X - tmp_data);
	next();
}

static void compare_Y(void) {
	puts("compare_Y");
	flag_c = (Y >= tmp_data);
	update_flags_ZN(Y - tmp_data);
	next();
}

static void bit_test(void) {
	puts("bit_test");
	flag_n = (tmp_data & 0x80);
	flag_v = (tmp_data & 0x40);
	flag_z = !(tmp_data & A);
	next();
}

/******************************************************** Branching ********************************************************/
static void check_flag_Z_set(void) {
	puts("check_flag_Z_set");
	if (!flag_z)
		next();
}

static void check_flag_N_set(void) {
	puts("check_flag_N_set");
	if (!flag_n)
		next();
}

static void check_flag_C_set(void) {
	puts("check_flag_C_set");
	if (!flag_c)
		next();
}

static void check_flag_V_set(void) {
	puts("check_flag_V_set");
	if (!flag_v)
		next();
}

static void check_flag_Z_clr(void) {
	puts("check_flag_Z_clr");
	if (flag_z)
		next();
}

static void check_flag_N_clr(void) {
	puts("check_flag_N_clr");
	if (flag_n)
		next();
}

static void check_flag_C_clr(void) {
	puts("check_flag_C_clr");
	if (flag_c)
		next();
}

static void check_flag_V_clr(void) {
	puts("check_flag_V_clr");
	if (flag_v)
		next();
}

static void branch_same_page(void) {
	puts("branch_same_page");
	//printf("  OFFSET %d\n", (signed char) tmp_data);
	effective_address = PC + (int8_t) tmp_data;
	if (PCH == effective_address_hi) {
		PC = effective_address;
		set_PC(PC);
		next();
	} else {
		PCL = effective_address_lo;
	}
}

static void branch_any_page(void) {
	puts("branch_any_page");
	PC = effective_address;
	set_PC(PC);
	next();
}

/********************************************************** Stack **********************************************************/
static void push_A(void) {
	puts("push_A");
	write_memory(0x100 | S--, A);
}

static void push_P(void) {
	puts("push_P");
	write_memory(0x100 | S--, group_status_flags());
}

static void pull_data(void) {
	puts("pull_data");
	tmp_data = read_memory(0x100 |++S);
}

static void pull_P(void) {
	puts("pull_P");
	ungroup_status_flags(read_memory(++S | 0x100));
}

static void push_PCH(void) {
	puts("push_PCH");
	write_memory(0x100 | S--, PCH);
}

static void pull_PCH(void) {
	puts("pull_PCH");
	effective_address_hi = read_memory(0x100 | ++S);
	set_PC(effective_address);
}

static void pull_PCH_minus_one(void) {
	puts("pull_PCH");
	effective_address_hi = read_memory(0x100 | ++S);
	set_PC(effective_address - 1);
}

static void push_PCL(void) {
	puts("push_PCL");
	write_memory(0x100 | S--, PCL);
}

static void pull_PCL(void) {
	puts("pull_PCL");
	effective_address = read_memory(++S | 0x100);
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

static void fetch_param_addr_hi_add_X(void) {
	puts("fetch_param_addr_hi_add_x");
	effective_address_hi = read_memory(PC);
	set_PC(PC + 1);
	if (effective_address_lo + X < 0x0100) {
		effective_address_lo += X;
		step++;
	}
}

static void fetch_param_addr_hi_add_Y(void) {
	puts("fetch_param_addr_hi_add_Y");
	effective_address_hi = read_memory(PC);
	set_PC(PC + 1);
	if (effective_address_lo + Y < 0x0100) {
		effective_address_lo += Y;
		step++;
	}
}

static void load_data(void) {
	puts("load_data");
#if DBG
	tmp_data = dbg_data;
	printf("tmp_data ->\033[1;43m %02X \033[0m\n", tmp_data);
#else
	tmp_data = read_memory(effective_address);
#endif
}

static void add_X_to_addr(void) {
	puts("add_X_to_addr");
	effective_address += X;
}

static void add_Y_to_addr(void) {
	puts("add_Y_to_addr");
	effective_address += Y;
}

static void add_X_to_addr_lo(void) {
	puts("add_X_to_addr_lo");
	effective_address_lo += X;
}

static void add_Y_to_addr_lo(void) {
	puts("add_Y_to_addr_lo");
	effective_address_lo += Y;
}

static void load_addr_lo(void) {
	puts("load_addr_lo");
	tmp_data = read_memory(effective_address);
}

static void load_addr_hi(void) {
	puts("load_addr_hi");
	effective_address_lo += 1; // JMP_ind and STA_indY do not cross page boundaries here
	effective_address_hi = read_memory(effective_address);
	effective_address_lo = tmp_data;
}

static void load_addr_hi_add_Y(void) {
	puts("load_addr_hi_add_Y");
	load_addr_hi();
	if (effective_address_lo + Y < 0x0100) {
		effective_address_lo += Y;
		step++;
	}
}

/***************************************************************************************************************************/
static void read_vector_L(void) {
	puts("read_vector_L");
	effective_address = read_memory(vector);
	flag_b = true;
}

static void read_vector_H(void) {
	puts("read_vector_H");
	effective_address_hi = read_memory(vector + 1);
	set_PC(effective_address);
}


opcode *RST_special    [] = { reset };
opcode *DMA_special    [] = { fetch_rubbish, wait_for_mem };
opcode *ERR_illegal    [] = { terminate };
opcode *BRK_stack      [] = { fetch_param_data, push_PCH, push_PCL, push_P, read_vector_L, read_vector_H, next };
opcode *RTI_stack      [] = { fetch_param_data, fetch_rubbish, pull_P, pull_PCL, pull_PCH_minus_one, next };

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

opcode *NOP_implied    [] = { fetch_rubbish, next };


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

opcode *AND_immediate  [] = { fetch_param_data, bitwise_and };
opcode *ORA_immediate  [] = { fetch_param_data, bitwise_or };
opcode *EOR_immediate  [] = { fetch_param_data, bitwise_xor };

opcode *ADC_immediate  [] = { fetch_param_data, add_with_carry };
opcode *SBC_immediate  [] = { fetch_param_data, subtract_with_carry };


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
opcode *STA_zeropage   [] = { fetch_param_addr_zp, store_A, next };
opcode *STX_zeropage   [] = { fetch_param_addr_zp, store_X, next };
opcode *STY_zeropage   [] = { fetch_param_addr_zp, store_Y, next };

opcode *LDA_zeropage   [] = { fetch_param_addr_zp, load_data, put_data_into_A };
opcode *LDX_zeropage   [] = { fetch_param_addr_zp, load_data, put_data_into_X };
opcode *LDY_zeropage   [] = { fetch_param_addr_zp, load_data, put_data_into_Y };

opcode *CMP_zeropage   [] = { fetch_param_addr_zp, load_data, compare_A };
opcode *CPX_zeropage   [] = { fetch_param_addr_zp, load_data, compare_X };
opcode *CPY_zeropage   [] = { fetch_param_addr_zp, load_data, compare_Y };
opcode *BIT_zeropage   [] = { fetch_param_addr_zp, load_data, bit_test };

opcode *AND_zeropage   [] = { fetch_param_addr_zp, load_data, bitwise_and };
opcode *ORA_zeropage   [] = { fetch_param_addr_zp, load_data, bitwise_or };
opcode *EOR_zeropage   [] = { fetch_param_addr_zp, load_data, bitwise_xor };

opcode *ADC_zeropage   [] = { fetch_param_addr_zp, load_data, add_with_carry };
opcode *SBC_zeropage   [] = { fetch_param_addr_zp, load_data, subtract_with_carry };

opcode *INC_zeropage   [] = { fetch_param_addr_zp, load_data, increment_data, store_data, next };
opcode *DEC_zeropage   [] = { fetch_param_addr_zp, load_data, decrement_data, store_data, next };

opcode *ASL_zeropage   [] = { fetch_param_addr_zp, load_data, shift_left_data, store_data, next };
opcode *LSR_zeropage   [] = { fetch_param_addr_zp, load_data, shift_right_data, store_data, next };
opcode *ROL_zeropage   [] = { fetch_param_addr_zp, load_data, rotate_left_data, store_data, next };
opcode *ROR_zeropage   [] = { fetch_param_addr_zp, load_data, rotate_right_data, store_data, next };


/* ABSOLUTE *******************************************************************/
opcode *STA_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, store_A, next };
opcode *STX_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, store_X, next };
opcode *STY_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, store_Y, next };

opcode *LDA_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, put_data_into_A };
opcode *LDX_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, put_data_into_X };
opcode *LDY_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, put_data_into_Y };

opcode *CMP_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, compare_A };
opcode *CPX_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, compare_X };
opcode *CPY_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, compare_Y };
opcode *BIT_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, bit_test };

opcode *AND_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, bitwise_and };
opcode *ORA_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, bitwise_or };
opcode *EOR_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, bitwise_xor };

opcode *ADC_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, add_with_carry };
opcode *SBC_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, subtract_with_carry };

opcode *INC_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, increment_data, store_data, next };
opcode *DEC_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, decrement_data, store_data, next };

opcode *ASL_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, shift_left_data, store_data, next };
opcode *LSR_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, shift_right_data, store_data, next };
opcode *ROL_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, rotate_left_data, store_data, next };
opcode *ROR_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_data, rotate_right_data, store_data, next };

opcode *JMP_absolute   [] = { fetch_param_addr_lo, fetch_param_addr_hi, branch_any_page };

opcode *JSR_absolute   [] = { fetch_param_addr_lo, fetch_rubbish, push_PCH, push_PCL, fetch_param_addr_hi, branch_any_page };


/* ZERO PAGE X ****************************************************************/
opcode *STA_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, store_A, next };
opcode *STY_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, store_Y, next };

opcode *LDA_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, put_data_into_A };
opcode *LDY_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, put_data_into_Y };

opcode *CMP_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, compare_A };

opcode *AND_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, bitwise_and };
opcode *ORA_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, bitwise_or };
opcode *EOR_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, bitwise_xor };

opcode *ADC_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, add_with_carry };
opcode *SBC_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, subtract_with_carry };

opcode *INC_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, increment_data, store_data, next };
opcode *DEC_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, decrement_data, store_data, next };

opcode *ASL_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, shift_left_data, store_data, next };
opcode *LSR_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, shift_right_data, store_data, next };
opcode *ROL_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, rotate_left_data, store_data, next };
opcode *ROR_zeropageX  [] = { fetch_param_addr_zp, add_X_to_addr_lo, load_data, rotate_right_data, store_data, next };


/* ZERO PAGE Y ****************************************************************/
opcode *STX_zeropageY  [] = { fetch_param_addr_zp, add_Y_to_addr_lo, store_X, next };

opcode *LDX_zeropageY  [] = { fetch_param_addr_zp, add_Y_to_addr_lo, load_data, put_data_into_X };


/* ABSOLUTE X *****************************************************************/
opcode *STA_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_X_to_addr, store_A, next };

opcode *LDA_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, put_data_into_A };
opcode *LDY_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, put_data_into_Y };

opcode *AND_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, bitwise_and };
opcode *ORA_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, bitwise_or };
opcode *EOR_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, bitwise_xor };

opcode *CMP_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, compare_A };

opcode *ADC_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, add_with_carry };
opcode *SBC_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_X, add_X_to_addr, load_data, subtract_with_carry };

opcode *INC_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_X_to_addr, load_data, increment_data,    store_data, next };
opcode *DEC_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_X_to_addr, load_data, decrement_data,    store_data, next };

opcode *ASL_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_X_to_addr, load_data, shift_left_data,   store_data, next };
opcode *LSR_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_X_to_addr, load_data, shift_right_data,  store_data, next };
opcode *ROL_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_X_to_addr, load_data, rotate_left_data,  store_data, next };
opcode *ROR_absoluteX  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_X_to_addr, load_data, rotate_right_data, store_data, next };


/* ABSOLUTE Y *****************************************************************/
opcode *STA_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi, add_Y_to_addr, store_A, next };

opcode *LDA_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, put_data_into_A };
opcode *LDX_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, put_data_into_X };

opcode *AND_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, bitwise_and };
opcode *ORA_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, bitwise_or };
opcode *EOR_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, bitwise_xor };

opcode *CMP_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, compare_A };

opcode *ADC_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, add_with_carry };
opcode *SBC_absoluteY  [] = { fetch_param_addr_lo, fetch_param_addr_hi_add_Y, add_Y_to_addr, load_data, subtract_with_carry };


/* INDIRECT X *****************************************************************/
//opcode *STA_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, store_A, next };

//opcode *LDA_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, load_data, put_data_into_A };

//opcode *AND_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, load_data, bitwise_and };
//opcode *ORA_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, load_data, bitwise_or };
//opcode *EOR_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, load_data, bitwise_xor };

//opcode *CMP_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, load_data, compare_A };
//
//opcode *ADC_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, load_data, add_with_carry };
//opcode *SBC_indirectX  [] = { fetch_param_addr_zp, add_X_to_addr, load_addr_lo, load_addr_hi, load_data, subtract_with_carry };


/* INDIRECT Y *****************************************************************/
opcode *STA_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi, add_Y_to_addr, store_A, next };

opcode *LDA_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi_add_Y, add_Y_to_addr, load_data, put_data_into_A };

opcode *AND_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi_add_Y, add_Y_to_addr, load_data, bitwise_and };
opcode *ORA_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi_add_Y, add_Y_to_addr, load_data, bitwise_or };
opcode *EOR_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi_add_Y, add_Y_to_addr, load_data, bitwise_xor };

opcode *CMP_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi_add_Y, add_Y_to_addr, load_data, compare_A };

opcode *ADC_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi_add_Y, add_Y_to_addr, load_data, add_with_carry };
opcode *SBC_indirectY  [] = { fetch_param_addr_zp, load_addr_lo, load_addr_hi_add_Y, add_Y_to_addr, load_data, subtract_with_carry };


/* INDIRECT *******************************************************************/
opcode *JMP_indirect   [] = { fetch_param_addr_lo, fetch_param_addr_hi, load_addr_lo, load_addr_hi, branch_any_page };


/* STACK **********************************************************************/
opcode *PHA_stack      [] = { fetch_rubbish, push_A, next };
opcode *PHP_stack      [] = { fetch_rubbish, push_P, next };

opcode *PLA_stack      [] = { fetch_rubbish, fetch_rubbish, pull_data, put_data_into_A };
opcode *PLP_stack      [] = { fetch_rubbish, fetch_rubbish, pull_data, put_data_into_P };

opcode *RTS_stack      [] = { fetch_param_data, fetch_rubbish, pull_PCL, pull_PCH, fetch_param_data, next };


opcode **oop[] = { ERR_illegal, SEI_implied };

