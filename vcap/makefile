INCLUDE = $(shell pkg-config opencv)

LIBS = $(shell pkg-config --libs opencv)

BIN = vcap

OBJECTS = vcap.o

SOURCES = vcap.cpp


$(BIN):$(OBJECTS)
	g++ -o $(BIN) $(OBJECTS) -I $(INCLUDE) $(LIBS) -static

$(OBJECTS):$(SOURCES)
	g++ -c $(SOURCES)

clean:
	rm $(OBJECTS) $(BIN) vcap_dump*
