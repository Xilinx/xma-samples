CC = gcc 
CFLAGS = $(shell pkg-config --cflags libxmaapi) 
CFLAGS += -Wall -O2 -g 
LDFLAGS = $(shell pkg-config --libs libxmaapi) 
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
