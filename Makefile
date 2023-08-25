all:
	gcc -o server server.c -lpthread
	gcc -o client client.c

strict:
	gcc -Wall -Werror -Wextra -O3 -o -server lab2plvN32481_server.c -lpthread
	gcc -Wall -Werror -Wextra -O3 -o client lab2plvN32481_client.c

clear:
	rm -rf *.o