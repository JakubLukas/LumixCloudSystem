#include "utils.h"

#include <random>


namespace CldSim
{


static std::mt19937& getRandomGenerator()
{
	static std::random_device seed;
	static std::mt19937 gen(seed());

	return gen;
}


float randFloat()
{
	std::uniform_real_distribution<float> dist;
	return dist(getRandomGenerator());
}


template<typename Type>
void createArray(uint size, Type** arrayLoc)
{
	*arrayLoc = new Type[size];//TODO: remove news, use allocator instead
	memset(*arrayLoc, 0, size * sizeof(Type));
}
template void createArray<bool>(uint size, bool** arrayLoc);
template void createArray<float>(uint size, float** arrayLoc);


}
