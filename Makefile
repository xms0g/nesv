TARGET=nesv.nes
CFG=conf/nrom_128_horz.cfg
OBJS=build/crt0.o build/riscv.o build/nesio.o build/dram.o build/bus.o build/main.o
CC=cc65
CA65=ca65
LD=ld65
CFLAGS=-Oirs --add-source
LDFLAGS=nes.lib -Ln build/labels.txt

all: build/$(TARGET)

src/%.s : src/%.c
	$(CC) $(CFLAGS) $< -o $@

build/%.o : src/%.s
	$(CA65) $< -o $@

build/%.o : libs/%.s
	$(CA65) $< -o $@
	
build/$(TARGET) : $(OBJS)
	$(LD) -C $(CFG) -o $@ $^ $(LDFLAGS) 

.PHONY: clean
clean:
	rm -f build/$(TARGET) $(OBJS)