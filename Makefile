build: main.c pbmedit.c
	gcc -Wall -Wextra main.c pbmedit.c -o image_editor -lm
clean:
	rm image_editor
.PHONY: clean
