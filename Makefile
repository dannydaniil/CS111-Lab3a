.SILENT:

TAR = lab3a-504636684.tar.gz
FILES = Makefile README lab3a.c
DEBUG= -g
FLAGS= -Wall
CC = gcc

default:  lab3a

lab3a:
	$(CC) $(DEBUG) $(FLAGS) lab3a.c -o lab3a

clean:
	rm -rf lab3a

dist:
	tar cfv $(TAR) $(FILES)
