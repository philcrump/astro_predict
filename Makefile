
CC = gcc
COPT = -O2 -march=native -mtune=native
CFLAGS += -Wall -Wextra -Wpedantic -Wunused -D_GNU_SOURCE

BIN = astro_predict

SRC = main.c \
	lunarRdPlan.c \
	stars.c

OBJ = ${SRC:.c=.o}

PALDIR = pal/
PALLIB = pal.a

ERFADIR = erfa/
ERFALIB = liberfa.a

LUNARDIR = lunar/
LUNARLIB = liblunar.a

LDFLAGS += -L${PALDIR} -l:${PALLIB} -L${ERFADIR}/src/.libs/ -l:${ERFALIB} -L${LUNARDIR} -l:${LUNARLIB} -lm

all: check-gitsubmodules ${BIN}

debug: COPT = -Og
debug: CFLAGS += -ggdb -fno-omit-frame-pointer
debug: all

werror: CFLAGS += -Werror
werror: all

$(BIN): ${OBJ}
	@echo "  LD     "$@
	@${CC} ${COPT} ${CFLAGS} ${OBJ} ${LDFLAGS} -o $@

%.o: %.c
	@echo "  CC     "$<
	@${CC} ${COPT} ${CFLAGS} -c -fPIC -o $@ $<

clean:
	@rm -rf ${BIN} ${OBJ}

check-gitsubmodules:
	@if git submodule status | egrep -q '^[-]|^[+]' ; then \
		echo "INFO: Need to [re]initialize git submodules"; \
		git submodule update --init; \
	fi

.PHONY: all clean check-gitsubmodules
