CC=g++

CFLAGS=-O9

DEST=.

BINS=$(DEST)/main


all: clean bin

bin: $(BINS)

main: 
	$(CC) $(CFLAGS) -o $(DEST)/main main.c -lm

clean:
	cd $(DEST); rm -f $(BINS)
