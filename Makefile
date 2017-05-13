CC     = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 libsoup-2.4`
LIBS   = `pkg-config --libs gtk+-3.0 libsoup-2.4`
FILES  = main.c util.c dv.c

build: $(FILES)
	$(CC) $(LIBS) $(CFLAGS) $(FILES) -Wall -g -rdynamic -o main

run: build
	./main | dvsource-file -p 1777 -h localhost -

test: build
	./main

clean:
	rm main
