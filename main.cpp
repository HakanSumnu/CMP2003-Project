#include "IBCF.hpp"
#include <iostream>
#include <random>

int main()
{
	IBCF ibcf;
	ibcf.readData("data/train.csv");
	std::cout << "Data analyzed" << std::endl;
	std::cout << std::endl;
	ibcf.printTop10();
	ibcf.predict("data/test.csv", "data/prediction.csv", 28);
	//std::random_device rd;
	//ibcf.test(20000, 28, rd());
	std::cout << "Finished" << std::endl;

	return 0;
}