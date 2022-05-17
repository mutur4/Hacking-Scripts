# compiler flags -s (make the binary stripped to make debugging harder) -static (make the library independent of libraries )

CC = gcc
CFLAGS = -s -static 

TARGET = pwnkit

$(TARGET): $(TARGET).c
  $(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
  
clean:
  $(RM) $(TARGET)
  
  

  
  
