a.out:
	g++ main.cpp

run: a.out
	@echo ""
	./a.out
	@echo "\n"
	@rm ./a.out

clear:
	rm ./*.ppm ./*.out