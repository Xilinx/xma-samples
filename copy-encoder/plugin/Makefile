CC = gcc 
CFLAGS = $(shell pkg-config --cflags libxma2plugin) 
CFLAGS += -fPIC -Wall -Wextra -O1 -g
LDFLAGS = -shared 
RM = rm -f
TARGET_LIB = libxlnxdatamover.so
SRCS = xlnx_datamover.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

