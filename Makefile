
# Makefile for drumk1 - a tiny tool to generate a
# mapping file for drumk1 from a folder full of wav files
# Frank Neumann, December 2013

CC=gcc
CFLAGS=-g -O3 -Wall -Werror
LDFLAGS=-g

all: drumkv1gen

drumk1gen: drumkv1gen.c
	$(CC)	$(CFLAGS) $< $(DLFAGS) -o $@

clean:
	rm -f drumkv1gen

