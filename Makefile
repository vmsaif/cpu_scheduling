all: 	clean cpuScheduling

cpuScheduling:
	gcc -fsanitize=address -g -Wall -Wpedantic -Wextra -Werror -o cpuScheduling cpuScheduling.c -lpthread
clean:
	rm -f *.o cpuScheduling
run:
	./cpuScheduling 1 1000 tasks.txt
	./cpuScheduling 1 3000 tasks.txt
	./cpuScheduling 1 5000 tasks.txt
	./cpuScheduling 1 7000 tasks.txt
	./cpuScheduling 2 1000 tasks.txt
	./cpuScheduling 2 3000 tasks.txt
	./cpuScheduling 2 5000 tasks.txt
	./cpuScheduling 2 7000 tasks.txt
	./cpuScheduling 3 1000 tasks.txt
	./cpuScheduling 3 3000 tasks.txt
	./cpuScheduling 3 5000 tasks.txt
	./cpuScheduling 3 7000 tasks.txt
	./cpuScheduling 4 1000 tasks.txt
	./cpuScheduling 4 3000 tasks.txt
	./cpuScheduling 4 5000 tasks.txt
	./cpuScheduling 4 7000 tasks.txt
