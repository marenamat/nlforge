all: nlforge interceptor.so

nlforge: nlforge.c nlstrings.h
	$(CC) -ggdb3 -lncurses -o $@ $<

interceptor.so: interceptor.c
	$(CC) -fPIC -shared -o $@ $< -ldl

.PHONY: all
