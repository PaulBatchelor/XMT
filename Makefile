.PHONY: clean

default: libxmt.a

CFLAGS = -Wall -pedantic -std=c89 -O2

OBJ = xmt.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

libxmt.a: $(OBJ)
	$(AR) rcs $@ $(OBJ)

clean:
	$(RM) $(OBJ)
	$(RM) libxmt.a
