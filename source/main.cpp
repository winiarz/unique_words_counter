#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <array>
#include <cstring>
#include <algorithm>
#include "constants.hpp"
#include "WordsRangesContainer.hpp"
using namespace std;

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

void sortRangeWithDuplicatesRemoval(WordCompressed* workspace, WordsRange& range)
{
	sort(workspace + range.start,
	     workspace + range.end);

	unsigned long long removed = 0; 
	for(unsigned long long i=range.start; i<range.end - removed - 1; i++)
	{
		if(workspace[i] == workspace[i+removed+1])
		{
			removed++;
		}
		workspace[i] = workspace[i+removed];
	}
	range.end -= removed;
}

void mergeRangesWithDuplicatesRemoval(WordCompressed* workspace, WordsRangeMergingParams mergingParams)
{
	unsigned long long rangeIdx1 = mergingParams.start1;
	unsigned long long rangeIdx2 = mergingParams.start2;
	unsigned long long tempWorkspaceSize = mergingParams.end1 - mergingParams.start1 +
	                                       mergingParams.end2 - mergingParams.start2;

	WordCompressed* tempWorkspace = new WordCompressed[tempWorkspaceSize];
	unsigned long long tempWorkspaceIdx = 0;

	while((rangeIdx1 < mergingParams.end1) && (rangeIdx2 < mergingParams.end2))
	{
		if(workspace[rangeIdx1] == workspace[rangeIdx2])
		{
			rangeIdx2++;
		}
		else if(workspace[rangeIdx1] < workspace[rangeIdx2])
		{
			tempWorkspace[tempWorkspaceIdx] = workspace[rangeIdx1];
			rangeIdx1++;
			tempWorkspaceIdx++;
		}
		else
		{
			tempWorkspace[tempWorkspaceIdx] = workspace[rangeIdx2];
			rangeIdx2++;
			tempWorkspaceIdx++;
		}
	}
	while(rangeIdx1 < mergingParams.end1)
	{
		tempWorkspace[tempWorkspaceIdx] = workspace[rangeIdx1];
		rangeIdx1++;
		tempWorkspaceIdx++;
	}
	while(rangeIdx2 < mergingParams.end2)
	{
		tempWorkspace[tempWorkspaceIdx] = workspace[rangeIdx2];
		rangeIdx2++;
		tempWorkspaceIdx++;
	}

	memcpy(workspace + mergingParams.start1, tempWorkspace, sizeof(WordCompressed)*tempWorkspaceIdx);
	mergingParams.resultRange.end = mergingParams.start1 + tempWorkspaceIdx;

	delete [] tempWorkspace;
}

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
