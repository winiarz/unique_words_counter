#include "merge.hpp"
#include "sort.hpp"
#include <cstring>

void mergeRangesWithDuplicatesRemoval(WordCompressed* workspace, WordsRangeMergingParams mergingParams)
{
	unsigned long long rangeIdx1 = mergingParams.start1;
	unsigned long long rangeIdx2 = mergingParams.start2;
	unsigned long long tempWorkspaceSize = mergingParams.end1 - mergingParams.start1 +
	                                       mergingParams.end2 - mergingParams.start2;

	try
	{
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
	catch(bad_alloc exception) // memory can't be allocated
	{
		memmove(&workspace[mergingParams.end1],
			    &workspace[mergingParams.start2],
				sizeof(WordCompressed)*(mergingParams.end2 - mergingParams.start2));
		mergingParams.resultRange.end = mergingParams.end1 + mergingParams.end2 - mergingParams.start2;

		sortRangeWithDuplicatesRemoval(workspace, mergingParams.resultRange);
	}
}

