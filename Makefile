INCLUDE:=-Iinclude \
        -I/usr/local/include/simple \
        -I/usr/include/libxml2 \
        -I/usr/local/include/event2 \
        -I/usr/local/include/zlog \
        -I/usr/local/include/cjson

SOURCE:=${wildcard source/*.c}
SOURCE+=${wildcard source/collect/*.c}
SOURCE+=${wildcard source/task/*.c}
SOURCE+=${wildcard source/rpc/*.c}
SOURCE+=${wildcard source/protocol/*.c}

CFLAGS:= -w#-fpermissive

LDFLAGS:=-L/usr/local/lib/ -lsp -levent -lxml2 -lzlog -lpthread -lcjson -lcurl

exe:
	gcc ${INCLUDE} ${SOURCE} ${CFLAGS} ${LDFLAGS} -o bin/collect.bin

all:
	make clean
	make exe

test:
	gcc -g ${INCLUDE} ${SOURCE} tests/test.c ${CFLAGS} ${LDFLAGS} -D_TEST_ -o tests/collect.test

clean:
	rm -rf bin/collect.bin
	rm -rf tests/collect.test
