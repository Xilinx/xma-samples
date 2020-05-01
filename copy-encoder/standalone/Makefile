CC = gcc 
CFLAGS = $(shell pkg-config --cflags libxma2api) 
CFLAGS += -Wall -O2 -g 
LDFLAGS = $(shell pkg-config --libs libxma2api) 
LDFLAGS += -lxrt_core -lpthread -ldl
RM = rm -f
TARGET = datamover_app

SRCS = datamover.c  
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET}

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CXXFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET} ${OBJS} $(SRCS:.c=.d) 
