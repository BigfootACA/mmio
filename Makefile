CFLAGS  ?= -g -flto -O4 -Wall -Wextra -Werror
LDFLAGS ?= -g -flto -O4
all: mmio
mmio.o: mmio.c
mmio: mmio.o
clean:
	rm -f mmio.o mmio
install: mmio
	install -vsDm755 mmio -t $(DESTDIR)/usr/bin
