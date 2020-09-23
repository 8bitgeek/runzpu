
TARGET=../bin/runzpu
CFLAGS+=-O2 -I./ -I../libzpu -I../libs19
libzpu=../libzpu/libzpu.a
LIBS19=../libs19/libs19.a

$(TARGET): runzpu.c 
	gcc runzpu.c $(CFLAGS) $(LIBS19) $(libzpu) -o $(TARGET) -l ncurses

clean:
	$(RM) *.o
	$(RM) *.lst
	$(RM) *.map
	$(RM) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

