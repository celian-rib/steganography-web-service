
main: main.c libspng/spng/spng.c
	gcc -g main.c libspng/spng/spng.c -l z -lm -o main

clean:
	rm -f main
