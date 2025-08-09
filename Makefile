TARGET=nesv.nes
CFG=config/nrom_128_horz.cfg
OBJS=build/riscv.o build/crt0.o build/main.o
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
	
build/$(TARGET) : $(OBJS)
	$(LD) -C $(CFG) -o $@ $^ $(LDFLAGS) 

.PHONY: clean
clean:
	rm -f build/$(TARGET) $(OBJS)