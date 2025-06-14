all : MS_Sans_Serif_18.c

%.c : %.yaff
	monobit-convert $< to --container-format=c -outfile=$@
