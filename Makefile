loss-sim:	main.o wav.o send.o recv.o dct.o
	gcc -g -Wall -o loss-sim main.o wav.o send.o recv.o dct.o -lm

main.o:	wav.h interpolate.h config.h main.c
	gcc -g -Wall -c main.c

wav.o:	wav.h interpolate.h config.h wav.c
	gcc -g -Wall -c wav.c
send.o:	wav.h interpolate.h config.h send.c
	gcc -g -Wall -c send.c
recv.o:	wav.h interpolate.h config.h recv.c
	gcc -g -Wall -c recv.c
dct.o:	wav.h interpolate.h config.h dct.c
	gcc -g -Wall -c dct.c
	
