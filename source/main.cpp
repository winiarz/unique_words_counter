#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include "constants.hpp"
#include "WordsRangesContainer.hpp"
#include "compressWord.hpp"
#include "sort.hpp"
#include "merge.hpp"
using namespace std;

atomic<bool> fileReadingDone = false;

void fileReading(string fileName,
                 WordsRangesContainer* rangesContainer,
				 WordCompressed* mainWorkspace)
{
	fstream file(fileName, fstream::in);
	while(not file.eof())
	{
		auto& readingRange = rangesContainer->createNewRangeForReading();
		if(readingRange.end == 0)
		{
			continue;
		}

		unsigned long long mainWorkspaceIdx = readingRange.start;
		string newWord;

		while((mainWorkspaceIdx<readingRange.end) and (file >> newWord))
		{
			mainWorkspace[mainWorkspaceIdx] = compressWord(newWord);
			mainWorkspaceIdx++;
		}
		readingRange.end = mainWorkspaceIdx;
		readingRange.isLocked = false;
	}

	file.close();
	fileReadingDone = true;
}

void sortAndMerge(WordsRangesContainer* rangesContainer,
				  WordCompressed* mainWorkspace)
{
	while(not fileReadingDone)
	{
		WordsRange* sortingRange = &rangesContainer->getRangeForSorting();
		if(sortingRange->end - sortingRange->start > 0)
		{
			sortRangeWithDuplicatesRemoval(mainWorkspace, *sortingRange);
			sortingRange->isSorted = true;
			sortingRange->isLocked = false;
			rangesContainer->markPossibleFreeSpace();

			continue;	
		}

		if(rangesContainer->areMultipleRanges())
		{
			WordsRangeMergingParams mergingParams = rangesContainer->prepareBestRangeForMerging();
			mergeRangesWithDuplicatesRemoval(mainWorkspace, mergingParams);
			mergingParams.resultRange.isLocked = false;
			rangesContainer->markPossibleFreeSpace();
			continue;
		}

		// TODO shall wait if there is nothing to do
	}

	WordsRange* sortingRange = &rangesContainer->getRangeForSorting();
	while(sortingRange->end - sortingRange->start > 0)
	{
		sortRangeWithDuplicatesRemoval(mainWorkspace, *sortingRange);
		sortingRange->isSorted = true;
		sortingRange->isLocked = false;
		rangesContainer->markPossibleFreeSpace();

		sortingRange = &rangesContainer->getRangeForSorting();
	}

	while(rangesContainer->areMultipleRanges())
	{
		WordsRangeMergingParams mergingParams = rangesContainer->prepareBestRangeForMerging();
		mergeRangesWithDuplicatesRemoval(mainWorkspace, mergingParams);
		mergingParams.resultRange.isLocked = false;
		rangesContainer->markPossibleFreeSpace();
	}
}


int main(int argc, char *argv[])
{
	WordCompressed* mainWorkspace = new WordCompressed[WORKSPACE_SIZE];
	if(argc <= 1)
	{
		cout << "Input filename expected" << endl;
	}

	string fileName(argv[1]);
	WordsRangesContainer rangesContainer;

	thread fileReadingThread(fileReading, fileName, &rangesContainer, mainWorkspace);
	vector<thread> workingThreads;
	for(unsigned int i=0; i<WORKING_THREADS; i++)
	{
		workingThreads.push_back(thread(sortAndMerge, &rangesContainer, mainWorkspace));
	}

	fileReadingThread.join();
	for(auto& workingThread : workingThreads)
		workingThread.join();
	cout << "There are " << rangesContainer.getSizeOfFirstRange() << " unique words in file " << fileName << endl;
	return 0;
}
