#include "WordsRangesContainer.hpp"
#include <iostream>

void WordsRangesContainer::printAllRanges()
{
	lock_guard accessLock(accessMutex);


	for(auto& range : wordsRanges)
	{
		cout << "[" << range.start << ", " << range.end << "]" << endl;
	}
}

WordsRange& WordsRangesContainer::createNewRangeForReading()
{
	{
	lock_guard accessLock(accessMutex);

	if(wordsRanges.empty())
	{
		wordsRanges.push_back(WordsRange(0, MAX_READ_SIZE));
		wordsRanges.begin()->isLocked = true;
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
			auto newRange = wordsRanges.insert(i, WordsRange(lastRangeEnd, lastRangeEnd+MAX_READ_SIZE));
			newRange->isLocked = true;
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
			wordsRanges.rbegin()->isLocked = true;
			return *wordsRanges.rbegin();
		}
		else if(foundRangeSize > maxFoundFreeSpace)
		{
			wordsRanges.push_back(WordsRange(lastRangeEnd, lastRangeEnd+foundRangeSize));
			wordsRanges.rbegin()->isLocked = true;
			return *wordsRanges.rbegin();
		}
	}

	if(maxFoundFreeSpace > 0)
	{
		auto newRange = wordsRanges.insert(maxFoundFreeSpaceIt, WordsRange(maxFoundFreeSpaceStart, maxFoundFreeSpaceStart+maxFoundFreeSpace));
		newRange->isLocked = true;
		return *newRange;
	}

	} // end of access lock

	possibleFreeSpaceForReading  = false;
	unique_lock<mutex> lk(freeSpaceForReading);
	freeSpaceForReadingCv.wait(lk, [&] {return possibleFreeSpaceForReading or workFinished;});
	return emptyRange;
}

WordsRange& WordsRangesContainer::getRangeForSorting()
{
	{
	lock_guard accessLock(accessMutex);

	if(workFinished)
	{
		return emptyRange;
	}

	for(auto i = wordsRanges.begin(); i!= wordsRanges.end(); i++)
	{
		if((not i->isSorted) and (not i->isLocked))
		{
			i->isLocked = true;
			return *i; 
		}
	}

	possibleSortWork  = false;
	} // end of access lock

	unique_lock<mutex> lk(sortWork);
	sortWorkCv.wait(lk, [&] {return possibleSortWork or workFinished;});

	return emptyRange;
}

WordsRangeMergingParams WordsRangesContainer::prepareBestRangeForMerging()
{
	lock_guard accessLock(accessMutex);

	if(wordsRanges.size() < 2)
	{
		possibleMergeWork = false;
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
		if((leftRangeToMerge->isSorted && rightRangeToMerge->isSorted) and
			(not leftRangeToMerge->isLocked && not rightRangeToMerge->isLocked))
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
		bestLeftRangeToMerge->isLocked = true;
		bestLeftRangeToMerge->end = bestRightRangeToMerge->end;
		wordsRanges.erase(bestRightRangeToMerge);
		return bestRangeMergingParams;
	}

	possibleMergeWork = false;
	WordsRangeMergingParams emptyMergingParams {emptyRange, 0,0,0,0};
	return emptyMergingParams;
}

unsigned long long WordsRangesContainer::getSizeOfFirstRange()
{
	lock_guard accessLock(accessMutex);


	if(wordsRanges.empty())
	{
		return 0;
	}

	return wordsRanges.begin()->end - wordsRanges.begin()->start;
}

bool WordsRangesContainer::areMultipleOrUnsortedRanges()
{
	lock_guard accessLock(accessMutex);
	if(wordsRanges.size() >= 2)
	{
		return true;
	}
	for(auto& range : wordsRanges)
	{
		if(not range.isSorted)
		{
			return true;
		}
	}

	return false;
}

void WordsRangesContainer::markPossibleFreeSpace()
{
	possibleFreeSpaceForReading=true;
	freeSpaceForReadingCv.notify_one();
}

void WordsRangesContainer::markPossibleMergeWork()
{
	possibleMergeWork=true;
	mergeWorkCv.notify_all();
}

void WordsRangesContainer::markPossibleSortWork()
{
	possibleSortWork=true;
	sortWorkCv.notify_all();
}

void WordsRangesContainer::notifyWorkFinished()
{
	workFinished = true;
	sortWorkCv.notify_all();
	mergeWorkCv.notify_all();
}

