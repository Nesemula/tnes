CC = gcc
CFLAGS = -O3 -Wall -Wextra -pedantic -march=native -std=c11 -flto -Winline

funestus : obj/core.o obj/cpu.o obj/ppu.o obj/memory.o obj/io.o -lSDL2 -lSDL2main
	$(CC) -s -flto $^ -o $@
	@ls -sh --color $@

obj/core.o : core.c common.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

obj/cpu.o : cpu.c opcode.c debug.h common.h
	$(CC) $(CFLAGS) -c $< -o $@ -Wno-unused-value

obj/ppu.o : ppu.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

obj/memory.o : memory.c common.h
	$(CC) $(CFLAGS) -c $< -o $@ -Wno-unused-value

obj/io.o : io.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm funestus obj/*

