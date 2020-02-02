// CPU
void cpu_exec(void);
void cpu_interrupt(void);
void cpu_hang(void);

// PPU
void ppu_setup(void);
void ppu_run(void);
void map_character_data(uint8_t *chr_data, uint8_t mirroring);
void ppu_write(uint16_t ppu_register, uint8_t data);
uint8_t ppu_read(uint16_t ppu_register);

// GUI
void sync(void);
void io_setup(void);
void display_frame(unsigned char *frame);
unsigned char get_input(void);
void reset_input(void);

// Loader
void load_ROM(const char *file_name);

// Memory
void map_program_data(unsigned char *prg_data);
unsigned char read_memory(unsigned short address);
void write_memory(unsigned short address, unsigned char data);
int memory_auto_transfer(void);

#define getchar 0&&getchar
#define puts 0&&puts
#define printf 0&&printf

