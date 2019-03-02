CC = gcc

# Compiler flags.
# For debugging
#  CDEBUG = -g
# For profiling
#  CDEBUG = -fno-inline -O4 -pg
# For optimization
CDEBUG = -O4

# Which addc() to use. Pick one.
# For C version
#   ADDC = -DADDC_C
# For asm version using jmp
ADDC = -DADDC_ASM_JMP

CFLAGS = -Wall -Wextra -Werror $(CDEBUG) $(ADDC)

OBJS = bigfib.o bigint.o bigadd.o unit.o
BINS = bigfib bigadd unit

all: $(BINS)

bigfib: bigfib.o bigint.o
	$(CC) $(CFLAGS) -o bigfib bigfib.o bigint.o

bigadd: bigadd.o bigint.o
	$(CC) $(CFLAGS) -o bigadd bigadd.o bigint.o

unit: unit.o bigint.o
	$(CC) $(CFLAGS) -o unit unit.o bigint.o

$(OBJS): bigint.h

clean:
	-rm -f $(OBJS) $(BINS) *.s gmon.out

.c.s:
	$(CC) $(CFLAGS) -S $*.c
