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