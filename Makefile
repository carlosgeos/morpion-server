FILE = rapport
DIR = tex_files
OPTIONS = -pdf -output-directory=$(DIR)
PDF_V = evince			#open, okular, skim, adobe...

auto: $(FILE).tex
	latexmk $(OPTIONS) $<

view: $(DIR)/$(FILE).pdf
	$(PDF_V) $< &

mac_view: $(DIR)/$(FILE).pdf
	open $< &
