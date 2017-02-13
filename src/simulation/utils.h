#pragma once


namespace CldSim
{


typedef unsigned char byte;
typedef unsigned int uint;


struct Vec3
{
	float x;
	float y;
	float z;


};


float randFloat();


template<typename Type>
void createArray(int size, Type** arrayLoc);


}
