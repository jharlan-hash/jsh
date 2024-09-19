default:
	clang jshell.c -o jsh
	./jsh

install:
	clang jshell.c -o jsh

clean:
	rm ./jsh

