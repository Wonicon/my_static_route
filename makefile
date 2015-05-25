CFILES = $(wildcard *.c)
myarp: $(CFILES)
	echo $(CFILES)
	gcc -Wall -Werror -ggdb -std=c99 $(CFILES) -o ../myarp

.PHONY: run
run: myarp
	sudo ../myarp
