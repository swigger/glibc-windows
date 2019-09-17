CC=/opt/llvm/bin/clang -target x86_64-pc-windows-musl -g
CXX=/opt/llvm/bin/clang -target x86_64-pc-windows-musl -g

all:seticase.exe uargs.exe mscl.exe

seticase.exe:seticase.o
	$(CC) -o $@ $^ -lc++_static

uargs.exe:uargs.o
	$(CC) -o $@ $^

mscl.exe:mscl.o
	$(CC) -o $@ $^ -lc++_static

clean:
	rm -f *.o *.pdb $(basename $(wildcard *.pdb)) *.exe


HOST=kswsl
run:
	ssh $(HOST) rm -f /mnt/d/mscl.exe
	rsync mscl.exe mscl.pdb /Users/xungeng/work.down/clang-win-musl/./build/musl/lib/libc.pdb /opt/llvm//x86_64-pc-windows-musl/lib/libc.so $(HOST):/mnt/d/
	ssh $(HOST) /mnt/d/mscl.exe
