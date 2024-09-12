default:
	clang jshell.c -o jsh
	./jsh

install:
	clang jshell.c -o jsh
	sudo cp jsh /usr/local/bin

clean:
	sudo rm /usr/local/bin/jsh
	rm ./jsh
