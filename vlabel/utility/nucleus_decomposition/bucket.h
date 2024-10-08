#ifndef __BUCKET__H
#define __BUCKET__H

#include <vector>
#include <unordered_map>
#include <stdlib.h>

struct Naive_Bucket_element
{
	Naive_Bucket_element * prev;
	Naive_Bucket_element * next;
	Naive_Bucket_element();
};

struct Naive_Bucket
{
	Naive_Bucket_element **buckets; 
	Naive_Bucket_element *elements; 
	int nb_elements;
	int max_value;
	int *values; 
	int current_min_value;

	Naive_Bucket();
	~Naive_Bucket();
	void Initialize(int max_value, int nb_element);
	void Free (); 
	void Insert(int id, int value);
	void Update(int id, int new_value);
	void DecVal(int id);
	int PopMin(int* ret_id, int* ret_value); 
	int CurrentValue(int id);
};

#endif
