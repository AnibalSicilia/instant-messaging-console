TARGET= imserver imclient
CC= gcc
OBJECTS1= imserver.o linked_list.o
OBJECTS2= imclient.o

all: $(TARGET)

%.o: %.c
	$(CC) -c $^
imserver: $(OBJECTS1)
	gcc -o $@ $^
imclient: $(OBJECTS2)
	gcc -o $@ $^

clean:
	rm -f $(TARGET) $(OBJECTS1) $(OBJECTS2) *~
