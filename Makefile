all : twmailer
twmailer : main.c
	gcc -Wall -O -o twmailer.out main.c
clean :
	rm -f twmailer.out