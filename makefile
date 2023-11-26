base:
	gcc -o tsp base.c utilities.c

advanced:
	gcc -o tsp advanced.c utilities.c
run:
	./tsp
clean:
	rm -r tsp
go:
	make build
	make run