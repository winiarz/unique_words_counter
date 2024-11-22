#include "compressWord.hpp"
#include <cstring>

WordCompressed compressWord(string& word)
{
	WordCompressed result;
	memset(result.data(), 0, sizeof(WordCompressed));

	for(int i=0; i<word.size(); i++)
	{
		result[i/6] = result[i/6] * SIGNS_NUMBER + (word[i]-'a');
	}

	return result;
}


