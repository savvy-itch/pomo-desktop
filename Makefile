CFLAGS += -g -O0
CFLAGS += $(shell pkg-config --cflags gtk4)
LDFLAGS = $(shell pkg-config --libs gtk4)

pomodoro: main.o filesave.o
	gcc -o pomodoro main.o filesave.o $(LDFLAGS)

main.o: main.c filesave.h
	gcc -c main.c $(CFLAGS)

filesave.o: filesave.c filesave.h
	gcc -c filesave.c $(CFLAGS)