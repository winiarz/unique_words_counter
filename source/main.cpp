#include <iostream>
#include <string>
#include <fstream>
#include <set>
using namespace std;

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
	set<string> wordsSet;

	while(file >> newWord)
	{
		counter++;
		wordsSet.insert(newWord);
	}
	file.close();

	cout << "There are " << counter << " words, unique words " << wordsSet.size() << " in file " << fileName << endl;
	return 0;
}
