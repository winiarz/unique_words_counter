#include "WordsRangesContainer.hpp"
#include <iostream>

void WordsRangesContainer::printAllRanges()
{
	for(auto& range : wordsRanges)
	{
		cout << "[" << range.start << ", " << range.end << "]" << endl;
	}
}

WordsRange& WordsRangesContainer::createNewRangeForReading()
{
	if(wordsRanges.empty())
	{
		wordsRanges.push_back(WordsRange(0, MAX_READ_SIZE));
		return *wordsRanges.begin();
	}

	unsigned long long maxFoundFreeSpace = 0;
	unsigned long long lastRangeEnd = 0;
	list<WordsRange>::iterator maxFoundFreeSpaceIt = wordsRanges.begin();
	unsigned long long maxFoundFreeSpaceStart = 0;

	for(auto i = wordsRanges.begin(); i!= wordsRanges.end(); i++)
	{
		unsigned long long foundRangeSize = i->start - lastRangeEnd;

		if(foundRangeSize >= MAX_READ_SIZE)
		{
			auto newRange = wordsRanges.insert(i, WordsRange(lastRangeEnd, MAX_READ_SIZE));
			return *newRange;
		}

		if(foundRangeSize > 0)
		{
			if(foundRangeSize > maxFoundFreeSpace)
			{
				maxFoundFreeSpace = foundRangeSize;
				maxFoundFreeSpaceStart = lastRangeEnd;
				maxFoundFreeSpaceIt = i;
			}
		}

		lastRangeEnd = i->end;
	}

	unsigned long long foundRangeSize =  WORKSPACE_SIZE - lastRangeEnd;
	if(foundRangeSize > 0)
	{
		if(foundRangeSize >= MAX_READ_SIZE)
		{
			wordsRanges.push_back(WordsRange(lastRangeEnd, lastRangeEnd+MAX_READ_SIZE));
			return *wordsRanges.rbegin();
		}
		else if(foundRangeSize > maxFoundFreeSpace)
		{
			wordsRanges.push_back(WordsRange(lastRangeEnd, lastRangeEnd+foundRangeSize));
			return *wordsRanges.rbegin();
		}
	}

	if(maxFoundFreeSpace > 0)
	{
		auto newRange = wordsRanges.insert(maxFoundFreeSpaceIt, WordsRange(maxFoundFreeSpaceStart, maxFoundFreeSpaceStart+maxFoundFreeSpace));
		return *newRange;
	}

	return emptyRange;
}

WordsRange& WordsRangesContainer::getRangeForSorting()
{
	for(auto i = wordsRanges.begin(); i!= wordsRanges.end(); i++)
	{
		if(not i->isSorted)
			return *i; 
	}

	return emptyRange;
}

WordsRangeMergingParams WordsRangesContainer::prepareBestRangeForMerging()
{
	if(wordsRanges.size() < 2)
	{
		WordsRangeMergingParams emptyMergingParams {emptyRange, 0,0,0,0};
		return emptyMergingParams;
	}

	auto leftRangeToMerge = wordsRanges.begin();
	auto rightRangeToMerge = ++wordsRanges.begin();

	unsigned long long shortestRangeToBeLocked = WORKSPACE_SIZE+1;
	list<WordsRange>::iterator bestLeftRangeToMerge = leftRangeToMerge;
	list<WordsRange>::iterator bestRightRangeToMerge = rightRangeToMerge; 

	for( ;  rightRangeToMerge != wordsRanges.end(); rightRangeToMerge++)
	{
		if(leftRangeToMerge->isSorted && rightRangeToMerge->isSorted)
		{
			unsigned long long rangeToBeLocked = rightRangeToMerge->end - leftRangeToMerge->start;
			if(rangeToBeLocked < shortestRangeToBeLocked)
			{
				shortestRangeToBeLocked = rangeToBeLocked;
				bestLeftRangeToMerge = leftRangeToMerge;
				bestRightRangeToMerge = rightRangeToMerge;
			}
		}
		leftRangeToMerge++;
	}

	if(shortestRangeToBeLocked <= WORKSPACE_SIZE)
	{
		WordsRangeMergingParams bestRangeMergingParams {*bestLeftRangeToMerge,
		                                                bestLeftRangeToMerge->start,
														bestLeftRangeToMerge->end,
														bestRightRangeToMerge->start,
														bestRightRangeToMerge->end};
		bestLeftRangeToMerge->end = bestRightRangeToMerge->end;
		wordsRanges.erase(bestRightRangeToMerge);
		return bestRangeMergingParams;
	}

	WordsRangeMergingParams emptyMergingParams {emptyRange, 0,0,0,0};
	return emptyMergingParams;
}

unsigned long long WordsRangesContainer::getSizeOfFirstRange()
{
	if(wordsRanges.empty())
	{
		return 0;
	}

	return wordsRanges.begin()->end - wordsRanges.begin()->start;
}