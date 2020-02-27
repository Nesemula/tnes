// CPU
void cpu_setup(uint8_t *prg_data, uint8_t prg_banks);
void cpu_exec(void);
void cpu_interrupt(void);
void cpu_hang(void);
void cpu_reset(void);

// PPU
void ppu_setup(uint8_t *chr_data, uint8_t chr_banks, uint8_t mirroring);
void ppu_run(void);
void ppu_write(uint16_t ppu_register, uint8_t data);
uint8_t ppu_read(uint16_t ppu_register);

// IO
void sync(void);
void io_setup(void);
void display_frame(unsigned char *frame);
unsigned char get_input(void);
void reset_input(void);

// Memory
unsigned char read_memory(unsigned short address);
void write_memory(unsigned short address, unsigned char data);
int memory_auto_transfer(void);

