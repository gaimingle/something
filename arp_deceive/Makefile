final = arp_deceive
objects = arp_deceive.o

$(final): $(objects)
	gcc -o $(final) $(objects) -lgm_net -lnet

.PHONY: clean
clean:
	rm $(final) $(objects)
