final = scan
object = scan.o

$(final): $(object)
	gcc -o $(final) $(object) -lgm_net

$(object): scan.h

.PHONY: clean
clean: 
	rm $(final) $(object)
