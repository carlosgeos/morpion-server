CC = gcc
FLAGS = -Wall -Wextra
FILE = rapport
DIR = tex_files
OPTIONS = -pdf -output-directory=$(DIR)
PDF_V = evince			#open, okular, skim, adobe...

compile: client.o server.o
client.o: client.c
	$(CC) $< -o $@

server.o: server.c
	$(CC) $< -o $@

# Run

connect: client.o
	./$< localhost

serve: server.o
	./$<

# LaTeX gen

tex: auto view

mac_tex: auto mac_view

auto: $(FILE).tex
	latexmk $(OPTIONS) $<

view: $(DIR)/$(FILE).pdf
	$(PDF_V) $< &

mac_view: $(DIR)/$(FILE).pdf
	open $< &
