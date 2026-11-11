CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDLIBS = -lm
FUENTES = main.c artemis.c validaciones.c

ifeq ($(OS),Windows_NT)
EJECUTABLE = artemis.exe
else
EJECUTABLE = artemis
endif

.PHONY: all clean
all: $(EJECUTABLE)

$(EJECUTABLE): $(FUENTES) artemis.h validaciones.h
	$(CC) $(CFLAGS) $(FUENTES) -o $(EJECUTABLE) $(LDLIBS)

clean:
ifeq ($(OS),Windows_NT)
	cmd /C "if exist $(EJECUTABLE) del /Q $(EJECUTABLE)"
else
	rm -f $(EJECUTABLE)
endif
