all:
	gcc -g mod.cpp -o mod.exe
	gcc -g p0.cpp -o p0.exe
	objdump -S /usr/lib/x86_64-linux-gnu/libc.a > libc.a.dis
	( cat libc.a.dis | perl g.pl ) > g.sh
	bash g.sh  | grep Mis
