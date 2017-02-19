#pragma once


namespace CldSim
{


typedef unsigned char byte;
typedef unsigned int uint;


float randFloat();


template<typename Type>
void createArray(uint size, Type** arrayLoc);


}
