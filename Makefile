.PHONY: all clean install uninstall

default: all

OBJ = base.o samples.o patterns.o instruments.o

%.o: %.c
	$(CC) -c $< -o $@

luaxmt.so: lua.c $(OBJ)
	gcc -Wall -shared -fPIC -o luaxmt.so  \
		-llua lua.c -lsndfile $(OBJ)
all:
	make $(OBJ) luaxmt.so
	cp luaxmt.so xmt.lua examples
clean:
	rm -rf $(OBJ) luaxmt.so
	rm -rf examples/*.xm
	rm -rf examples/luaxmt.so examples/xmt.lua

install: luaxmt.so 
	cp luaxmt.so xmt.lua /usr/local/lib/lua/5.1

uninstall:
	cd /usr/local/lib/lua/5.1; rm luaxmt.so xmt.lua	
