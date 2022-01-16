LIBS=-lcsfml-graphics -lcsfml-window -lcsfml-system -lcsfml-audio

_DEPS=main.c xochip.h display.h emulator.h buzzer.h
DEPS=$(patsubst %,src/%,$(_DEPS))

CC=gcc
CFLAGS=-I src

ODIR=obj
_OBJ=main.o xochip.o display.o emulator.o buzzer.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

xochip.out: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(ODIR)/*.o xochip.out

