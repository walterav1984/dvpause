CC     = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 libsoup-2.4`
LIBS   = `pkg-config --libs gtk+-3.0 libsoup-2.4`
FILES  = main.c util.c dv.c

build: $(FILES)
	$(CC) $(FILES) $(LIBS) $(CFLAGS) -Wall -g -rdynamic -o main

run: build
	./main | dvsource-file -p 1234 -h 127.0.0.1 -

test: build
	./main

clean:
	rm main
