prez.nes: crt0.o prez.o nrom_128_horz.cfg
	ld65 -C nrom_128_horz.cfg -o prez.nes crt0.o prez.o runtime.lib

prez.s: prez.c
	cc65 -Oi prez.c --add-source

prez.o: prez.s
	ca65 prez.s

crt0.o: prez.chr
	ca65 crt0.s

clean:
	rm prez.nes crt0.o prez.o prez.s

start: prez.nes
	fceux prez.nes
