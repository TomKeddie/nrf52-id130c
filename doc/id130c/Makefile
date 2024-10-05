PREFIX=$(subst .kicad_pro,,$(shell ls -1 *.kicad_pro))

all: schematic

schematic: $(PREFIX).kicad_sch.pdf

%.kicad_sch.pdf : %.kicad_sch
	kicad-cli sch export pdf $< -o $@
