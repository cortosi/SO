CFLAGS = -std=c89 -pedantic
TARGET = master
OBJ    = Utilities.o Semx.o Master.o Handling.o


$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

all: $(TARGET)


clean:	
	rm -f *.o $(TARGET) *~

run: $(TARGET)
	./$(TARGET)
