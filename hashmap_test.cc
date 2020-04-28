#include "lib/concurrent_unordered_map.h"

#include <iostream>

int main() {
	ConcurrentUnorderedMap<int, std::string> map;
	std::cout << map.insert(1, "heheh") << std::endl;
	std::cout << map.get(1) << std::endl;
	std::cout << map.size() << std::endl;
	std::cout << map.update(1, "sdsd") << std::endl;
	std::cout << map.get(1) << std::endl;
	std::cout << map.remove(1) << std::endl;
	std::cout << map.size() << std::endl;
}