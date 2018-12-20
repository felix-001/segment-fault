rm crash
echo compile ...
arm-linux-gnueabihf-gcc -finstrument-functions -funwind-tables catch_segfault.c segmentfault.c -o crash
#arm-linux-gnueabihf-gcc  -funwind-tables segfault.c seg-fault-dump-registers.c -o seg-fault-dump-registers
echo "copy to /home/work/share"
cp crash /home/work/share/

echo "disamble ..."
arm-linux-gnueabihf-objdump -S crash > dis-crash.s
