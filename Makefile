SJASMPLUS := sjasmplus
SALVADOR := salvador
WINE := wine
MDSLINK := mdslink

MDSDATA := $(wildcard data/bgm/*.mml) $(wildcard data/bgm/*.mds) $(wildcard data/se/*.mml) $(wildcard data/se/*.mds)

.PHONY: all pre-build mdsdrv mdsdata demo sgdk-demo clean

all: pre-build mdsdrv mdsdata demo sgdk-demo

clean:
	cd out; rm -f mdssub.bin mdssub.zx0 mdsdrv.bin mdsseq.bin mdspcm.bin mdsseq.inc mdsseq.h main.bin
	$(MAKE) -C sample/sgdk clean
	cd sample/sgdk/res; rm -f mdsdrv.bin mdsseq.bin mdspcm.bin mdsseq.h

sgdk-demo: pre-build mdsdata mdsdrv
	cd out; cp mdsdrv.bin mdsseq.bin mdspcm.bin mdsseq.h ../sample/sgdk/res
	$(MAKE) -C sample/sgdk

demo: pre-build out/main.bin

out/main.bin: src/main.68k mdsdata mdsdrv
	$(WINE) tools/asm68k.exe /k /p /o ae- $<,$@

mdsdata: pre-build out/mdsseq.bin

out/mdsseq.bin: $(MDSDATA)
	$(MDSLINK) -o $@ out/mdspcm.bin -i out/mdsseq.inc -h out/mdsseq.h $^

mdsdrv: pre-build out/mdsdrv.bin

out/mdsdrv.bin: src/blob.68k out/mdssub.zx0
	$(WINE) tools/asm68k.exe /k /p /o ae- $<,$@

out/mdssub.zx0: out/mdssub.bin
	$(SALVADOR) $< $@

out/mdssub.bin: src/mdssub.z80
	$(SJASMPLUS) $< --raw=$@

pre-build:
	mkdir -p out
