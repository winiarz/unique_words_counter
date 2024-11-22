#include <iostream>
#include <string>
#include <fstream>
#include "constants.hpp"
#include "WordsRangesContainer.hpp"
#include "compressWord.hpp"
#include "sort.hpp"
#include "merge.hpp"
using namespace std;


int main(int argc, char *argv[])
{
	WordCompressed* mainWorkspace = new WordCompressed[WORKSPACE_SIZE];
	if(argc <= 1)
	{
		cout << "Input filename expected" << endl;
	}

	string fileName(argv[1]);
	string newWord;
	fstream file(fileName, fstream::in);
	
	WordsRangesContainer rangesContainer;
	while(not file.eof())
	{
		auto& readingRange = rangesContainer.createNewRangeForReading();

		unsigned long long mainWorkspaceIdx = readingRange.start;
		while((mainWorkspaceIdx<readingRange.end) and (file >> newWord))
		{
			mainWorkspace[mainWorkspaceIdx] = compressWord(newWord);
			mainWorkspaceIdx++;
		}
		readingRange.end = mainWorkspaceIdx;
	}
	file.close();

	WordsRange* sortingRange = &rangesContainer.getRangeForSorting();
	while(sortingRange->end - sortingRange->start > 0)
	{
		sortRangeWithDuplicatesRemoval(mainWorkspace, *sortingRange);
		sortingRange->isSorted = true;

		sortingRange = &rangesContainer.getRangeForSorting();
	}

	while(rangesContainer.areMultipleRanges())
	{
		WordsRangeMergingParams mergingParams = rangesContainer.prepareBestRangeForMerging();
		mergeRangesWithDuplicatesRemoval(mainWorkspace, mergingParams);
	}

	
	cout << "There are " << rangesContainer.getSizeOfFirstRange() << " unique words in file " << fileName << endl;
	return 0;
}
