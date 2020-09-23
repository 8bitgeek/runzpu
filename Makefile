TARGET=dummy

$(TARGET):
	(cd libzpu && make)
	(cd libs19 && make)
	(cd src && make)
	(cd test && make)

clean:
	(cd libzpu && make clean)
	(cd libs19 && make clean)
	(cd src && make clean)
	(cd test && make clean)

install:
	(cd libzpu && make install)
	(cd libs19 && make install)
	(cd src && make install)

