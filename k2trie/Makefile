CC=g++

CFLAGS=-O9

DEST=.

BINS=$(DEST)/project


all: clean bin

bin: $(BINS)

project: 
	$(CC) $(CFLAGS) -o $(DEST)/project main.c -lm

clean:
	cd $(DEST); rm -f $(BINS)
