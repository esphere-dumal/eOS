dd if=./bin/mbr.bin of=./bochs/hd60M.img bs=512 count=1 conv=notrunc

dd if=./bin/loader.bin of=./bochs/hd60M.img bs=512 count=1 seek=2 conv=notrunc