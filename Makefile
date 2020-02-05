CC = gcc
FLAGS = -O3 -Wall -Wextra -pedantic -march=native -flto

funestus : obj/core.o obj/cpu.o obj/ppu.o obj/memory.o obj/loader.o obj/io.o -lSDL2 -lSDL2main
	$(CC) -s $^ -o $@
	@ls -sh --color $@

obj/core.o : core.c common.h
	@mkdir -p obj
	$(CC) $(FLAGS) -c $< -o $@

obj/cpu.o : cpu.c opcode.c debug.h common.h
	$(CC) $(FLAGS) -c $< -o $@ -Wno-unused-value

obj/ppu.o : ppu.c common.h
	$(CC) $(FLAGS) -c $< -o $@

obj/memory.o : memory.c common.h
	$(CC) $(FLAGS) -c $< -o $@

obj/loader.o : loader.c common.h
	$(CC) $(FLAGS) -c $< -o $@

obj/io.o : io.c common.h
	$(CC) $(FLAGS) -c $< -o $@

clean :
	rm funestus obj/*

