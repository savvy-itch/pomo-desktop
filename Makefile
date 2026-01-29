CFLAGS += -g -O0 -Wall
CFLAGS += $(shell pkg-config --cflags gtk4)
LDFLAGS = $(shell pkg-config --libs gtk4)

pomodoro: main.o filesave.o settings-dialog.o pomodoro.gresource.o
	gcc -o pomodoro main.o filesave.o settings-dialog.o pomodoro.gresource.o $(LDFLAGS)

main.o: main.c filesave.h settings-dialog.h
	gcc -c main.c $(CFLAGS)

filesave.o: filesave.c filesave.h
	gcc -c filesave.c $(CFLAGS)

settings-dialog.o: settings-dialog.c settings-dialog.h
	gcc -c settings-dialog.c $(CFLAGS)

pomodoro.gresource.o: pomodoro.gresource.c
	gcc -c pomodoro.gresource.c $(CFLAGS)

pomodoro.gresource.c: pomodoro.gresource.xml settings.ui
	glib-compile-resources pomodoro.gresource.xml \
		--generate-source \
		--target=$@ \
		--sourcedir=.