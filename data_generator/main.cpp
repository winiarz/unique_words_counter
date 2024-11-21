#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
using namespace std;

constexpr unsigned long long minWordLen = 3;
constexpr unsigned long long maxWordLen = 10;
constexpr int randSeed = 123456;

int main(int argc, char* argv[])
{
	if(argc <= 2)
	{
		cout << "2 arguments expected:" << endl;
		cout << "#1 filename" << endl;
		cout << "#2 size of generated data in Bytes" << endl;
		return 1;
	}

	string fileName(argv[1]);
	string dataSizeStr(argv[2]);
	unsigned long long dataSize = 0;
	
	try
	{
		dataSize = stoull(dataSizeStr);
	}
	catch(...)
	{
		cout << "'" << dataSizeStr << "' is not correct data size" << endl;
	}
	

	fstream file(fileName, fstream::out);
	unsigned long long dataGenerated = 0;
	unsigned long long wordsGenerated = 0;

	srand(randSeed);
	while(dataGenerated < dataSize)
	{
		unsigned long long newWordLen = minWordLen + rand()%(maxWordLen - minWordLen);

		string newWord(newWordLen, 'a');

		for(int i=0; i<newWordLen; i++)
		{
			newWord[i] = 'a' + rand() % ('z' - 'a');
		}

		if(rand()%10 == 0)
			file << newWord << "\n";
		else
			file << newWord << " ";

		dataGenerated += newWordLen+1;
		wordsGenerated++;
	}
	file.close();

	cout << "Generated " << wordsGenerated << " with total length (including whitespaces) of " << dataGenerated << endl;

	return 0;
}

