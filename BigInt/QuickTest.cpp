#include "BigInt.h"
#include <vector>
#include <iostream>

int main() {
	BigInt a{ std::vector<unsigned>{4294967295,4294967295,1}, 0};
	BigInt b{ std::vector<unsigned>{4294967295,0,1}, 0};

	BigInt c = a - b;
	c.simplePrint();

	BigInt d{ "12345" };
	d.simplePrint();
}