all: tiffProcessor

tiffProcessor: 131044019_main.o
	gcc 131044019_main.o -o tiffProcessor

131044019_main.o: 131044019_main.c
	gcc -c 131044019_main.c

clean:
	rm *.o tiffProcessor
