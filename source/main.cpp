#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <array>
#include <cstring>
using namespace std;

constexpr unsigned int MAX_WORD_LEN = 24;
constexpr unsigned int SIGNS_NUMBER = 'z'-'a';
typedef  array<unsigned int, MAX_WORD_LEN/6> WordCompressed;


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

int main(int argc, char *argv[])
{

	if(argc <= 1)
	{
		cout << "Input filename expected" << endl;
	}

	string fileName(argv[1]);
	fstream file(fileName, fstream::in);

	string newWord;
	int counter=0;
	set<WordCompressed> wordsSet;

	while(file >> newWord)
	{
		counter++;
		wordsSet.insert(compressWord(newWord));
	}

	file.close();

	cout << "There are " << counter << " words, unique words " << wordsSet.size() << " in file " << fileName << endl;
	return 0;
}
