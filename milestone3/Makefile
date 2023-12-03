zip:
	zip milestone3.zip *.c *.h

all: main.c sbuffer.c sbuffer.h
	gcc *.c -o main.out -lpthread

memtest: all
	valgrind --leak-check=yes ./main.out

clean:
	rm -fr *.o