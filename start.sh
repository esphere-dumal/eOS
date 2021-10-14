nasm -I boot -o ./bin/mbr.bin boot/mbr.S 
nasm -I boot -o ./bin/loader.bin boot/loader.S
dd if=./bin/mbr.bin of=./bochs/hd60M.img bs=512 count=1 conv=notrunc
dd if=./bin/loader.bin of=./bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc


gcc -m32 -I ./lib/kernel/ -I ./lib/ -c -fno-builtin -o ./bin/main.o ./kernel/main.c
nasm -f elf -o ./bin/print.o  ./lib/kernel/print.S
nasm -f elf -o ./bin/kernel.o ./kernel/kernel.S
gcc -I ./lib/kernel -I ./lib -I ./kernel -c -fno-builtin -o ./bin/interrupt.o ./kernel/interrupt.c
gcc -I ./lib/kernel -I ./lib -I ./kernel -c -fno-builtin -o ./bin/init.o ./kernel/init.c

ld -m elf_i386 -Ttext 0xc0001500 -e main -o ./bin/kernel.bin \
./bin/main.o ./bin/print.o ./bin/kernel.o ./bin/interrupt.o ./bin/init.o

dd if=./bin/kernel.bin of=./bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc

bochs -f bochs/bochs.disk