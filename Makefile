build: main.c pbmedit.c
	gcc -Wall -Wextra main.c pbmedit.c -o image_editor -lm
clean:
	rm image_editor
pack:
	zip -FSr 314CA_Tudor-Daniel_Condrea_Tema3.zip README Makefile main.c pbmedit.c pbmedit.h
.PHONY: clean pack
