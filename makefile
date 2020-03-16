CC = gcc -lpthread
CFLAGS =
TARGET = master
TARGET1 = bin_adder
OBJ = master.o
OBJ1 = bin_adder.o
SRC = master.c
SRC1 = bin_adder.c
all: $(TARGET) $(TARGET1)
$(TARGET):$(OBJ)
	$(CC) -o $(TARGET) $(OBJ) -lm
$(TARGET1):$(OBJ1)
	$(CC) -o $(TARGET1) $(OBJ1) -lm
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)
$(OBJ1): $(SRC1)
	$(CC)  $(CFLAGS) -c $(SRC1)
clean:
	/bin/rm -f *.o $(TARGET)
	/bin/rm -f *.o $(TARGET1)

