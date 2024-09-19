default:
	clang jshell.c -o jssh -lreadline
	./jssh

clean:
	rm ./jssh

