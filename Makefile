CC = clang
CFLAGS = -Wall -Wextra -std=c11
TARGET = compiler
OBJS = main.o symboltable.o token.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

main.o: main.c symboltable.h token.h
	$(CC) $(CFLAGS) -c main.c

symboltable.o: symboltable.c symboltable.h
	$(CC) $(CFLAGS) -c symboltable.c

token.o: token.c token.h
	$(CC) $(CFLAGS) -c token.c

clean:
	rm -f $(OBJS) $(TARGET) temp.lex

.PHONY: all clean

run: $(TARGET)
	./$(TARGET)test_data/max/Max.asm output.hack