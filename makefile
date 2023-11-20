go:
	make build
	make run

build:
	gcc -o main main.c
run:
	./tsp $(ARGS)
clean:
	rm -r main