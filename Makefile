CC = gcc
CFLAGS = -Wall -g

SRCS = main.c parser.c executor.c builtins.c utils.c ai_helper.c
OBJS = $(SRCS:.c=.o)
TARGET = mysh

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
