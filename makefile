go:
	make build
	make run

build:
	gcc -o main main.c
run:
	./main
clean:
	rm -r main