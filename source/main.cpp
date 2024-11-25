#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include <set>
#include "constants.hpp"
#include "WordsRangesContainer.hpp"
#include "compressWord.hpp"
#include "sort.hpp"
#include "merge.hpp"
using namespace std;

atomic<bool> fileReadingDone = false;

void fileReading(string fileName,
                 WordsRangesContainer* rangesContainer,
				 WordCompressed* mainWorkspace,
				 set<string>* longWordsSet)
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
			if(newWord.size() <= MAX_WORD_LEN)
			{
				mainWorkspace[mainWorkspaceIdx] = compressWord(newWord);
				mainWorkspaceIdx++;
			}
			else
			{
				longWordsSet->insert(newWord);
			}
		}
		readingRange.end = mainWorkspaceIdx;
		readingRange.isLocked = false;
		rangesContainer->markPossibleSortWork();
	}

	file.close();
	fileReadingDone = true;
}

void sorting(WordsRangesContainer* rangesContainer,
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
			rangesContainer->markPossibleMergeWork();
		}
	}

	WordsRange* sortingRange = &rangesContainer->getRangeForSorting();
	while(sortingRange->end - sortingRange->start > 0)
	{
		sortRangeWithDuplicatesRemoval(mainWorkspace, *sortingRange);
		sortingRange->isSorted = true;
		sortingRange->isLocked = false;
		rangesContainer->markPossibleFreeSpace();
		rangesContainer->markPossibleMergeWork();

		sortingRange = &rangesContainer->getRangeForSorting();
	}
}


void merging(WordsRangesContainer* rangesContainer,
			 WordCompressed* mainWorkspace)
{
	while(not fileReadingDone)
	{
		rangesContainer->waitForMergingWork();
		
		WordsRangeMergingParams mergingParams = rangesContainer->prepareBestRangeForMerging();
			
		if(mergingParams.end2 > 0)
		{
			mergeRangesWithDuplicatesRemoval(mainWorkspace, mergingParams);
			mergingParams.resultRange.isLocked = false;
			rangesContainer->markPossibleFreeSpace();
		}
	}

	while(rangesContainer->areMultipleOrUnsortedRanges())
	{
		rangesContainer->waitForMergingWork();
		WordsRangeMergingParams mergingParams = rangesContainer->prepareBestRangeForMerging();
		if(mergingParams.end2>0)
		{
			mergeRangesWithDuplicatesRemoval(mainWorkspace, mergingParams);
			mergingParams.resultRange.isLocked = false;
			rangesContainer->markPossibleFreeSpace();
		}
	}

	rangesContainer->notifyWorkFinished();
}


int main(int argc, char *argv[])
{
	WordCompressed* mainWorkspace = new WordCompressed[WORKSPACE_SIZE];
	set<string> longWordsSet;
	if(argc <= 1)
	{
		cout << "Input filename expected" << endl;
	}

	string fileName(argv[1]);
	WordsRangesContainer rangesContainer;

	thread fileReadingThread(fileReading, fileName, &rangesContainer, mainWorkspace, &longWordsSet);
	vector<thread> workingThreads;
	for(unsigned int i=0; i<WORKING_THREADS; i++)
	{
		workingThreads.push_back(thread(sorting, &rangesContainer, mainWorkspace));
	}
	for(unsigned int i=0; i<WORKING_THREADS; i++)
	{
		workingThreads.push_back(thread(merging, &rangesContainer, mainWorkspace));
	}

	fileReadingThread.join();
	for(auto& workingThread : workingThreads)
		workingThread.join();
	unsigned long long totalUniqueWordsNb = rangesContainer.getSizeOfFirstRange()+longWordsSet.size();
	cout << "There are " << totalUniqueWordsNb << " unique words in file " << fileName << endl;
	return 0;
}
