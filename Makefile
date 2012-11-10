CC := msp430-gcc
CFLAGS := -mmcu=msp430f2013 -Wall -pedantic -C90 -Os
DEBUG := mspdebug
DFLAGS := rf2500
LINT := splint
LINTFLAGS := -I /usr/msp430/include -D__MSP430_IOMACROS_H_= \
   "-Dsfrb(x,x_)=volatile extern unsigned char x" \
   "-Dsfrw(x,x_)=volatile extern unsigned int x" \
   "-Dconst_sfrb(x,x_)=volatile extern const unsigned char x" \
   "-Dconst_sfrw(x,x_)=volatile extern const unsigned int x" \
   -checks -exportheader +boolint +charint -namechecks

TARGETNAME := mspuart
SRC := main.c

#-include $(SRC:%.c=%.d)

all: lint $(TARGETNAME).elf

.PHONY : lint
lint:
	$(LINT) $(LINTFLAGS) $(SRC)

$(TARGETNAME).elf: $(SRC) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRC)
	msp430-objdump -h $@ | egrep "text|bss" | awk '{printf "%5s %s\n", $$2, $$3}'

program: $(TARGETNAME).elf
	echo prog $^ | $(DEBUG) $(DFLAGS)

.PHONY: debug
debug:
	$(DEBUG) $(DFLAGS)
