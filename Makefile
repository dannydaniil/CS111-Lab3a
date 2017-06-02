.SILENT:

TAR = lab3b-504636684.tar.gz
FILES = Makefile README lab3b.py
CC = python

default:  lab3a

lab3a:
	$(CC) lab3a.py

clean:
	rm -rf lab3a report.csv

dist:
	tar cfv $(TAR) $(FILES)
