#pragma once

struct User_Node
{
	int user_id;
	double rating;
	User_Node* next;
};

struct Item
{
	int capacity;
	User_Node** user_hashmap;
};

class IBCF
{
	Item* items;
	int* users;
	double* average_ratings_of_users;
	double* euclidean_norms_of_items;
	int item_hashmap_size;
	int user_hashmap_size;

	int itemHashFunction(int);
	int userHashFunction(int);
	int userHashFunction(int, int);

public:
	IBCF();
	~IBCF();

	void readData(const char*);
	void printTop10();
	void predict(const char*, const char*, int);
	void test(int, int, int);
};