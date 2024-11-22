#include "sort.hpp"
#include <algorithm>

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


