nasm -I boot -o ./bin/mbr.bin boot/mbr.S 
nasm -I boot -o ./bin/loader.bin boot/loader.S
dd if=./bin/mbr.bin of=./bochs/hd60M.img bs=512 count=1 conv=notrunc
dd if=./bin/loader.bin of=./bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc

mkdir ./bin/obj
nasm -f elf -o ./bin/obj/print.o  ./lib/kernel/print.S
nasm -f elf -o ./bin/obj/kernel.o ./kernel/kernel.S
gcc -m32 -I ./lib/kernel/ -I ./lib/ -c -fno-builtin -o ./bin/obj/main.o ./kernel/main.c
gcc -m32 -I ./lib/kernel -I ./lib -I ./kernel -c -fno-builtin -fno-stack-protector -o ./bin/obj/interrupt.o ./kernel/interrupt.c
gcc -m32 -I ./lib/kernel -I ./lib -I ./kernel -c -fno-builtin -fno-stack-protector -o ./bin/obj/init.o ./kernel/init.c

ld -m elf_i386 -Ttext 0xc0001500 -e main -o ./bin/kernel.bin \
./bin/obj/main.o ./bin/obj/print.o ./bin/obj/kernel.o ./bin/obj/interrupt.o ./bin/obj/init.o

dd if=./bin/kernel.bin of=./bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc

bochs -f bochs/bochs.disk