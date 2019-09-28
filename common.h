// CPU
void cpu_exec(void);
void cpu_interrupt(void);
void cpu_dma(unsigned char base_address);

// PPU
void ppu_exec(void);
void map_character_data(unsigned char *data);
void ppu_write(unsigned short ppu_register, unsigned char data);
unsigned char ppu_read(unsigned short ppu_register);
void force_display(void);

// GUI
void initialize_display(void);
void display_frame(unsigned char *frame);

// Loader
void load_ROM(const char *file_name);

// Memory
void map_program_data(unsigned char *prg_data);
unsigned char read_memory(unsigned short address);
void write_memory(unsigned short address, unsigned char data);

#ifdef NOPAUSE
#define getchar 0&&getchar
#endif

#ifdef NOPRINT
#define printf 0&&printf
#define puts 0&&puts
#endif

