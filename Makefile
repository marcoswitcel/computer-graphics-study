a.out:
	g++ main.cpp

run: a.out
	@echo ""
	./a.out
	@echo "\n"
	@rm ./a.out

run-and-display: run
	xdg-open *.ppm

clear:
	rm ./*.ppm ./*.out

profile:
	g++ -ggdb -g -pg -O0 main.cpp
	gprof
