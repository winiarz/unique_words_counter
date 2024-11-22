#pragma once
#include <list>
#include "constants.hpp"

using namespace std;

struct WordsRange {
WordsRange(unsigned long long p_start, unsigned long long p_end) :
	start(p_start), end(p_end), isSorted(false)
	{}

unsigned long long start, end;
bool isSorted;
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
public:
	WordsRangesContainer() :
		emptyRange(0,0) {}

	void printAllRanges();
	WordsRange& createNewRangeForReading();
	WordsRange& getRangeForSorting();
	WordsRangeMergingParams prepareBestRangeForMerging();
	bool areMultipleRanges() {return wordsRanges.size()>=2;}
	unsigned long long getSizeOfFirstRange();
};

