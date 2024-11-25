#pragma once
#include <list>
#include <mutex>
#include <condition_variable>
#include "constants.hpp"

using namespace std;

struct WordsRange {
WordsRange(unsigned long long p_start, unsigned long long p_end) :
	start(p_start), end(p_end), isSorted(false), isLocked(false)
	{}

unsigned long long start, end;
bool isSorted, isLocked;
};

struct WordsRangeMergingParams {
WordsRange& resultRange;
unsigned long long start1, end1;
unsigned long long start2, end2;
};

class WordsRangesContainer {
private:
	list<WordsRange> wordsRanges;
	WordsRange emptyRange;

	mutex accessMutex;
	
	mutex freeSpaceForReading;
	condition_variable freeSpaceForReadingCv;
	bool possibleFreeSpaceForReading;

	mutex sortWork;
	condition_variable sortWorkCv;
	bool possibleSortWork;

	mutex mergeWork;
	condition_variable mergeWorkCv;
	bool possibleMergeWork;

	bool workFinished;
public:
	WordsRangesContainer() :
		emptyRange(0,0),
		possibleFreeSpaceForReading(true),
		possibleSortWork(false),
		possibleMergeWork(false),
		workFinished(false)
		{}

	void printAllRanges();
	WordsRange& createNewRangeForReading();
	WordsRange& getRangeForSorting();
	WordsRangeMergingParams prepareBestRangeForMerging();

	void waitForMergingWork()
	{
		if(possibleMergeWork or workFinished) return;
		if(areMultipleOrUnsortedRanges()) return;
		unique_lock<mutex> lk(mergeWork);
		mergeWorkCv.wait(lk, [&] {return possibleMergeWork or workFinished;});
	}
	unsigned long long getSizeOfFirstRange();
	void markPossibleFreeSpace();
	void markPossibleMergeWork();
	void markPossibleSortWork();
	void notifyWorkFinished();
	bool areMultipleOrUnsortedRanges();
};

