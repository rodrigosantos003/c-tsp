base:
	gcc -o tsp base.c

advanced:
	gcc -o tsp advanced.c
run:
	./tsp
clean:
	rm -r tsp
go:
	make build
	make run