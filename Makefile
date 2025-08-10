CC = gcc
CFLAGS = -ansi -pedantic -Wall -Wextra -g
OBJ = assembler.o utils.o pre_assembler.o first_assembler.o tables_utils.o second_assembler.o
TARGET = assembler

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<
