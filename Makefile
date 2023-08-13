.PHONY: clean

#default: libxmt.a
default: xmt

CFLAGS = -Wall -pedantic -std=c89 -O2 -DXMT_MAIN

# OBJ = xmt.o
OBJ = parse.o obj.o moncmp.o cmp/cmp.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# libxmt.a: $(OBJ)
# 	$(AR) rcs $@ $(OBJ)

xmt: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(OBJ)
	$(RM) libxmt.a
