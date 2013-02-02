//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Searching and sorting algorithms
//-----------------------------------------------------------------------------

#pragma once

/*
 Odporuca sa pouzivat QuickSort. Ak by hrozil stack overflow tak HeapSort.

 Poradie rychlosti algoritmov je priblizne taketo: quick, heap, shell, CombSort11, bubble.
 Ak niektore funkcie (zatial len quicksort) pozaduju ako vstupne parametre dolnu a
 hornu hranicu, tak pri volani sa zadavaju parametre (0, field_length-1)

 Priklad pouzitia v programe:
 HeapSort<int> (FieldToSort, FieldLength);
 QuickSort<float> (FieldToSort, 0, FieldLength - 1)
 ...pouzivat datove typy, ktore maju funkcie porovnania <>=, teda float, int a podobne
*/


// velmi rychla a ucinna hashovacia funkcia pre text
// retazec *str musi byt ukonceny nulou!
//------------------------------------------
inline unsigned int StringHashFunc (const char *str)
{
	unsigned int intHash = 0;
	if (str == NULL) return 0;

	for (register DWORD i=0; str[i]; ++i)
		intHash = str[i] + (intHash << 6) + (intHash << 16) - intHash;

	return intHash;
}


// vyhladava hodnotu searchValue v poli sortedField, sortedField musi byt vzostupne utriedene
// vrati index pola, ak sa hodnota nenajde vrati -1
//------------------------------------------
template<class T> int BinarySearch (T sortedField[], int fieldSize, T searchValue)
{
	int middle;
	int left = 0;
	int right = fieldSize-1;

	while (left <= right)
	{
		middle = (left + right) / 2;
		if (sortedField[middle] < searchValue)
			left = middle+1;
		else if (sortedField[middle] > searchValue)
			right = middle-1;
		else
			return middle;
	}
	return -1;
}

// rychly algoritmus s pouzitim rekurzie
#define QSORT_WITH_BUBBLE
//------------------------------------------
template<class T> void QuickSort (T a[], int lo0, int hi0)
{
	int lo = lo0;
	int hi = hi0;

#ifdef QSORT_WITH_BUBBLE
	if ((hi-lo) <= 6)
	{
		for (int j=hi; j > lo; j--)
			for (int i=lo; i < j; i++)
				if (a[i] > a[i+1])
				{
					T Temp = a[i];
					a[i] = a[i+1];
					a[i+1] = Temp;
				}
		return;
	}
#else
	if (lo >= hi)
		return;
	else if( lo == hi - 1 )		// sort a two element list by swapping if necessary
	{
		if (a[lo] > a[hi])
		{
			T Temp = a[lo];
			a[lo] = a[hi];
			a[hi] = Temp;
		}
		return;
	}
#endif

	// Pick a pivot and move it out of the way
	int Idx = (lo + hi) / 2;
	T pivot = a[Idx];
	a[Idx] = a[hi];
	a[hi] = pivot;

	while (lo < hi)
	{
		// Search forward from a[lo] until an element is found that is greater than the pivot or lo >= hi
		while ((a[lo] <= pivot) && (lo < hi))
			lo++;
		// Search backward from a[hi] until element is found that is less than the pivot, or lo >= hi
		while ((pivot <= a[hi]) && (lo < hi))
			hi--;
		// Swap elements a[lo] and a[hi]
		if (lo < hi)
		{
			T Temp = a[lo];
			a[lo] = a[hi];
			a[hi] = Temp;
		}
	}

	// Put the median in the "center" of the list
	a[hi0] = a[hi];
	a[hi] = pivot;

	/* Recursive calls, elements a[lo0] to a[lo-1] are less than or
	equal to pivot, elements a[hi+1] to a[hi0] are greater than pivot. */
	QuickSort (a, lo0, lo-1);
	QuickSort (a, hi+1, hi0);
}

// rychly algoritmus, bez rekurzie
//------------------------------------------
template<class T> inline void downheap(T a[], int k, int N)
{
	T Temp = a[k - 1];
	while (k <= N/2)
	{
		int j = k + k;
		if ((j < N) && (a[j - 1] < a[j]))
			j++;
		if (Temp >= a[j - 1])
			break;
		else
		{
			a[k - 1] = a[j - 1];
			k = j;
		}
	}
	a[k - 1] = Temp;
}

template<class T> void HeapSort (T a[], int ItemCount)
{
	int N = ItemCount;
	for (int k = N/2; k > 0; k--)
		downheap<T> (a, k, N);
	do
	{
		T Temp = a[0];
		a[0] = a[N - 1];
		a[N - 1] = Temp;
		N = N - 1;
		downheap<T> (a, 1, N);
	}while (N > 1);
}

// pomerne rychly algoritmus, asi 2-krat pomalsi ako quicksort, nepouziva rekurziu
// tipec rychlejsi ako CombSort11
//------------------------------------------
template<class T> void ShellSort (T a[], int ItemCount)
{
	int h = 1;
	/* find the largest h value possible */
	while ((h * 3 + 1) < ItemCount)
		h = 3 * h + 1;

	/* while h remains larger than 0 */
	while( h > 0 )
	{
		/* for each set of elements (there are h sets) */
		for (int i = h - 1; i < ItemCount; i++)
		{
			/* * pick the last element in the set */
			T B = a[i];
			int j = i;
			/*
			* compare the element at B to the one before it in the set
			* if they are out of order continue this loop, moving
			* elements "back" to make room for B to be inserted.
			*/
			for( j = i; (j >= h) && (a[j-h] > B); j -= h)
				a[j] = a[j-h];
			/* insert B into the correct place */
			a[j] = B;
		}
		/* all sets h-sorted, now decrease set size */
		h = h / 3;
	}
}

// dost pomaly algoritmus, quicksort moze byt aj 1000-krat rychlejsi...hlavne pri vacsom pocte prvkov
// ale napr. na uz (z vacsej casti) utriedene pole je bubble rychlejsi ako quick... inak neodporucam pouzivat
//------------------------------------------
template<class T> void BidirectBubbleSort (T a[], int ItemCount)
{
	T			Temp;
	boolean	flipped;
	int		limit = ItemCount;
	int		st = -1;

	while (st < limit)
	{
		flipped = false;
		st++;
		limit--;
		for (register int j = st; j < limit; j++)
		{
			if (a[j] > a[j + 1]) {
				Temp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = Temp;
				flipped = true;
			}
		}
		if (!flipped) return;
		flipped = false;

		for (register int j = limit; --j >= st;)
		{
			if (a[j] > a[j + 1])
			{
				Temp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = Temp;
				flipped = true;
			}
		}
		if (!flipped) return;
	}
}

/*
// pomerne rychly algoritmus, asi 2-krat pomalsi ako quicksort, nepouziva rekurziu
#define SHRINKFACTOR		1.3f
//------------------------------------------
template<class T> void CombSort11 (T a[], int ItemCount)
{
	boolean flipped = false;
	int gap, top;
	int i, j;

	gap = ItemCount;
	do {
		gap = (int) ((float) gap / SHRINKFACTOR);
		switch (gap) {
			case 0: / * the smallest gap is 1 - bubble sort * /
				gap = 1;
				break;
			case 9: / * this is what makes this Combsort11 * /
			case 10: 
				gap = 11;
				break;
			default: break;
		}
		flipped = false;
		top = ItemCount - gap;
		for (i = 0; i < top; i++)
		{
			j = i + gap;
			if (a[i] > a[j])
			{
				T Temp = a[i];
				a[i] = a[j];
				a[j] = Temp;
				flipped = true;
			}
		}
	} while (flipped || (gap > 1));
	/ * like the bubble and shell sorts we check for a clean pass * /
}
*/