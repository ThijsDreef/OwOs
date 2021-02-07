#include <inttypes.h>
#include "util/font.h"
#include <stdio.h>
#include "util/font.inc"

const character_t* getCharacter(char character) {
	int position = character - 33;
	if (position < 0 || position > sizeof(characters) / sizeof(character_t)) return NULL;
	return &characters[position];
}

int binarySearchKerning(char lookFor, int l, int r) {
	if (r >= 1) {
		int mid = l + (r - l) / 2;
		if (kernings[mid].first == lookFor)
			return mid;
		
		if (kernings[mid].first > lookFor)
			return binarySearchKerning(lookFor, l, mid - 1);
		
		return binarySearchKerning(lookFor, mid + 1, r);
	}
	return -1;
}

const kerning_t* getKerning(char first, char second) {
	int position = binarySearchKerning(first, 0, sizeof(kernings) / sizeof(kerning_t));
	if (position == -1) return NULL;
	const kerning_t* kerning = &kernings[position];
	if (kerning->second == second) return kerning;
	
	int direction = (kerning->second > second) ? -1 : 1;
	while (kerning->first == first) {
		kerning += direction;
		if (kerning->second == second)
			return kerning;
	}
	return NULL;
}

const void* getFontPage(int page) {
	return image[page];
}