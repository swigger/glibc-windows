CC=/opt/llvm/bin/clang -target x86_64-pc-windows-musl -g
CXX=/opt/llvm/bin/clang -target x86_64-pc-windows-musl -g

all:seticase.exe uargs.exe

seticase.exe:seticase.o
	$(CC) -o $@ $^ -lc++_static

uargs.exe:uargs.o
	$(CC) -o $@ $^

clean:
	rm -f *.o *.pdb $(basename $(wildcard *.pdb)) *.exe
