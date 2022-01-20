LIBS=-lcsfml-graphics -lcsfml-window -lcsfml-system -lcsfml-audio

_DEPS=main.c xochip.h display.h buzzer.h fontsets.h
DEPS=$(patsubst %,src/%,$(_DEPS))

CC=gcc
CFLAGS=-I src -O2

ODIR=obj
_OBJ=main.o xochip.o display.o buzzer.o fontsets.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

xochip.out: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(ODIR)/*.o xochip.out

