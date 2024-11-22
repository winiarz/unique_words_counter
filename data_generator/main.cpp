#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <random>
using namespace std;

constexpr unsigned long long minWordLen = 3;
constexpr unsigned long long maxWordLen = 10;
constexpr int randSeed = 123456;

string generateRandomWord()
{
	unsigned long long newWordLen = minWordLen + rand()%(maxWordLen - minWordLen);

	string newWord(newWordLen, 'a');

	for(int i=0; i<newWordLen; i++)
	{
		newWord[i] = 'a' + rand() % ('z' - 'a');
	}

	return newWord;
}

int main(int argc, char* argv[])
{
	srand(randSeed);

	if(argc <= 3)
	{
		cout << "3 arguments expected:" << endl;
		cout << "#1 filename" << endl;
		cout << "#2 mode: 'random' generates random words, if provided with number n instead generates n, then generates text with n unique words" << endl;
		cout << "#3 size of generated data in Bytes" << endl;
		return 1;
	}

	string fileName(argv[1]);
	string modeOrUniqueSize(argv[2]);
	string dataSizeStr(argv[3]);
	unsigned long long dataSize = 0;

	try
	{
		dataSize = stoull(dataSizeStr);
	}
	catch(...)
	{
		cout << "'" << dataSizeStr << "' is not correct data size" << endl;
		return 1;
	}

	fstream file(fileName, fstream::out);

	unsigned long long dataGenerated = 0;
	unsigned long long wordsGenerated = 0;

	if(modeOrUniqueSize.compare("random")==0)
	{
		while(dataGenerated < dataSize)
		{
			
			string newWord = generateRandomWord();

			if(rand()%10 == 0)
				file << newWord << "\n";
			else
				file << newWord << " ";

			dataGenerated += newWord.size()+1;
			wordsGenerated++;
		}
	}
	else
	{
		unsigned long long uniqueSize=0;
		try
		{
			uniqueSize = stoull(modeOrUniqueSize);
		}
		catch(...)
		{
			cout << "'" << modeOrUniqueSize << "' is not correct number" << endl;
		}

		set<string> randomWords;
		while(randomWords.size() < uniqueSize)
			randomWords.insert(generateRandomWord());

		while(dataGenerated < dataSize)
		{
			vector<string> wordSample;
			sample(randomWords.begin(),
			       randomWords.end(),
				   std::back_inserter(wordSample),
                   1,
				   std::mt19937{std::random_device{}()});

			if(rand()%10 == 0)
				file << *wordSample.begin() << "\n";
			else
				file << *wordSample.begin() << " ";

			dataGenerated += wordSample.begin()->size() + 1;
			wordsGenerated++;
		}
	}

	file.close();


	cout << "Generated " << wordsGenerated << " with total length (including whitespaces) of " << dataGenerated << endl;

	return 0;
}

