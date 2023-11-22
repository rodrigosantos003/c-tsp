build:
	gcc -o tsp main.c
run:
	./tsp
clean:
	rm -r tsp
go:
	make build
	make run