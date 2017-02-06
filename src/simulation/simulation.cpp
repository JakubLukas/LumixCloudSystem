#include "simulation.h"

#include <string>
#include <stdlib.h>


template<typename Type>
void createArray(int size, Type** arrayLoc)
{
	*arrayLoc = new Type[size];//TODO: remove news, use allocator instead
	memset(*arrayLoc, 0, size * sizeof(Type));
}



Simulation::Simulation()
	: m_width(0)
	, m_height(0)
	, m_length(0)
	, m_elapsedSteps(0)
	, m_actualIndex(0)
{
	m_densitySpace[0] = nullptr;
	m_densitySpace[1] = nullptr;
}


Simulation::~Simulation()
{
	Clear();
}


bool Simulation::Setup(uint width, uint height, uint length)
{
	m_width = width;
	m_height = height;
	m_length = length;

	int size = width * height * length;
	m_actualIndex = 0;
	m_elapsedSteps = 0;

	createArray<float>(size, &m_densitySpace[0]);
	createArray<float>(size, &m_densitySpace[1]);

	return true;
}


void Simulation::Clear()
{
	delete[] m_densitySpace[0];
	delete[] m_densitySpace[1];
}


void Simulation::Update(float delta_time)
{
	for(int x = 0; x < m_width; ++x)
	{
		for(int y = 0; y < m_height; ++y)
		{
			for(int z = 0; z < m_length; ++z)
			{
				Simulate(x, y, z);
			}
		}
	}

	m_actualIndex = 1 - m_actualIndex;
}


void Simulation::Simulate(int x, int y, int z)
{
	int neighborCount = GetNeighborCount(m_densitySpace[1 - m_actualIndex], x, y, z, 0.5f);

	int index = GetIndex(x, y, z);

	if(neighborCount <= 1)
		m_densitySpace[m_actualIndex][index] = 0.0f;
	else if (neighborCount == 5)
		m_densitySpace[m_actualIndex][index] = 1.0f;
	else if (neighborCount >= 8)
		m_densitySpace[m_actualIndex][index] = 0.0f;

	float random = rand() / (float)RAND_MAX;
	if(random > 0.9f)
		m_densitySpace[m_actualIndex][index] = 1.0f;
}



int Simulation::GetIndex(int x, int y, int z) const
{
	return (x * m_width * m_height + y * m_height + z);
}


bool Simulation::IsPointInSpace(const Vec3& point) const
{
	return (point.x >= 0) && (point.x < m_width)
		&& (point.y >= 0) && (point.y < m_height)
		&& (point.z >= 0) && (point.z < m_length);
}


bool Simulation::IsCellInSpace(int i, int j, int k) const
{
	return (i >= 0) && (i < m_width)
		&& (j >= 0) && (j < m_height)
		&& (k >= 0) && (k < m_length);
}


template<typename Type>
int Simulation::GetNeighborCount(Type* space, int x, int y, int z, Type valueGreaterThan) const
{
	int index = 0;
	int neighbors = 0;

	for(int i = -1; i <= 1; ++i)
	{
		for(int j = -1; j <= 1; ++j)
		{
			for(int k = -1; k <= 1; ++k)
			{
				if(i == 0 && j == 0 && k == 0)
					continue;

				index = GetIndex(x + i , y + j, z + k);
				if(IsCellInSpace(x + i, y + j, z + k) && space[index] > valueGreaterThan)
					++neighbors;
			}
		}
	}

	return neighbors;
}
