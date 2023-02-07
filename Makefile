all: myshell
myshell: main.c
	gcc -o myshell main.c
clean:
	rm -f myshell