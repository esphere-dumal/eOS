nasm -I boot -o ./bin/mbr.bin boot/mbr.S 
nasm -I boot -o ./bin/loader.bin boot/loader.S
dd if=./bin/mbr.bin of=./bochs/hd60M.img bs=512 count=1 conv=notrunc
dd if=./bin/loader.bin of=./bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc

nasm -f elf -o ./lib/kernel/print.o ./lib/kernel/print.S

gcc -m32 -I ./lib/kernel/ -I ./lib/ -c -o ./kernel/main.o ./kernel/main.c

ld -m elf_i386 ./kernel/main.o -Ttext 0xc0001500 -e main -o ./kernel/kernel.bin ./lib/kernel/print.o

dd if=./kernel/kernel.bin of=./bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc

bochs -f bochs/bochs.disk