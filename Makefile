all:
	gcc -std=gnu99 -Wall -Werror fat16.c main.c -o fat16
debug:
	gcc -std=gnu99 -Wall -Werror fat16.c main.c -o fat16 -g
clean:
	rm -f fat16