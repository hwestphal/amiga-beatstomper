all: dist/BeatStomperIII

dist/BeatStomperIII: src/bs.c src/midi.c src/midi.h
	vc -dontwarn=72 -lamiga -o dist/BeatStomperIII src/bs.c src/midi.c

clean:
	rm -f dist/BeatStomperIII
