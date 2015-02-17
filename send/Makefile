final = send
objects = send.o

$(final): $(objects)
	gcc -o $(final) $(objects) -lgm_net

$(objects): send.h

.PHONY: clean
clean:
	rm $(final) $(objects)
