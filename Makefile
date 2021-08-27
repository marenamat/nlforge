.PHONY: all clean

all: nlforge interceptor.so

nlforge: nlforge.c nlstrings.h
	$(CC) -ggdb3 -o $@ $< -lncurses -lpcap

interceptor.so: interceptor.c
	$(CC) -fPIC -shared -o $@ $< -ldl

clean:
	rm -f nlforge interceptor.so core
