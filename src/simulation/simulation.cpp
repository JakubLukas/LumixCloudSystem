#include "simulation.h"


namespace CldSim
{


Simulation::Simulation()
	: m_width(0)
	, m_height(0)
	, m_length(0)
	, m_elapsedSteps(0)
	, m_actualIndex(0)
{
	m_hum[0] = nullptr;
	m_hum[1] = nullptr;

	m_act[0] = nullptr;
	m_act[1] = nullptr;

	m_cld[0] = nullptr;
	m_cld[1] = nullptr;

	m_ext[0] = nullptr;
	m_ext[1] = nullptr;
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

	createArray<bool>(size, &m_hum[0]);
	createArray<bool>(size, &m_hum[1]);

	createArray<bool>(size, &m_act[0]);
	createArray<bool>(size, &m_act[1]);

	createArray<bool>(size, &m_cld[0]);
	createArray<bool>(size, &m_cld[1]);

	createArray<bool>(size, &m_ext[0]);
	createArray<bool>(size, &m_ext[1]);

	createArray<float>(size, &m_extTimes);

	InitValues();

	return true;
}


void Simulation::InitValues()
{
	int size = m_width * m_height * m_length;

	for(int i = 0; i < size; ++i)
	{
		m_hum[1 - m_actualIndex][i] = randFloat() < m_pHumInit;
		m_act[1 - m_actualIndex][i] = m_hum[1 - m_actualIndex][i] && (randFloat() < m_pActInit);
		m_ext[1 - m_actualIndex][i] = 0;
		m_cld[1 - m_actualIndex][i] = 0;
		m_extTimes[i] = 0.0f;
	}
}


void Simulation::Clear()
{
	delete[] m_hum[0];
	delete[] m_hum[1];

	delete[] m_act[0];
	delete[] m_act[1];

	delete[] m_cld[0];
	delete[] m_cld[1];

	delete[] m_ext[0];
	delete[] m_ext[1];

	delete[] m_extTimes;
}


void Simulation::Restart()
{
	InitValues();
}


void Simulation::Update(float deltaTime)
{
	/*static float timePassed = 0.0f;
	static const float STEP = 1.0f;
	timePassed += deltaTime;
	if (timePassed < STEP)
		return;

	timePassed -= STEP;*/

	for (int x = 0; x < m_width; ++x)
	{
		for (int y = 0; y < m_height; ++y)
		{
			for (int z = 0; z < m_length; ++z)
			{
				Simulate(x, y, z, deltaTime);
			}
		}
	}

	m_actualIndex = 1 - m_actualIndex;
}


void Simulation::Simulate(int x, int y, int z, float deltaTime)
{
	int index = GetIndex(x, y, z);

	m_act[m_actualIndex][index] =
		(!m_act[1 - m_actualIndex][index]
		&& m_hum[1 - m_actualIndex][index]
		&& CalcNeighborFunc(m_act[1 - m_actualIndex], x, y, z))
		|| (randFloat() < m_pActUpdate);

	m_hum[m_actualIndex][index] =
		(m_hum[1 - m_actualIndex][index]
		&& !m_act[1 - m_actualIndex][index])
		|| (randFloat() < m_pHumUpdate);

	m_ext[m_actualIndex][index] =
		(!m_ext[1 - m_actualIndex][index]
		&& m_cld[1 - m_actualIndex][index]
		&& CalcNeighborFunc(m_ext[1 - m_actualIndex], x, y, z))
		|| (randFloat() < m_pExtUpdate);

	bool newCld = !m_ext[1 - m_actualIndex][index]
		&& (m_cld[1 - m_actualIndex][index] || m_act[1 - m_actualIndex][index]);

	if (!newCld && m_cld[m_actualIndex][index])
	{
		m_extTimes[index] += deltaTime;
		if(m_extTimes[index] > m_tExt)
			m_extTimes[index] = 0.0f;
		else
			newCld = !newCld;
	}

	m_cld[m_actualIndex][index] = newCld;
}



int Simulation::GetIndex(int x, int y, int z) const
{
	return (x * m_width * m_height + y * m_height + z);
}


bool Simulation::IsCellInSpace(int i, int j, int k) const
{
	return (i >= 0) && (i < m_width)
		&& (j >= 0) && (j < m_height)
		&& (k >= 0) && (k < m_length);
}


bool Simulation::CalcNeighborFunc(bool* space, int x, int y, int z) const
{
	int index;
	bool result = false;

	if (IsCellInSpace(x + 1, y, z))
	{
		index = GetIndex(x + 1, y, z);
		result |= space[index];
	}

	if (IsCellInSpace(x - 1, y, z))
	{
		index = GetIndex(x - 1, y, z);
		result |= space[index];
	}

	if (IsCellInSpace(x, y - 1, z))
	{
		index = GetIndex(x, y - 1, z);
		result |= space[index];
	}

	if (IsCellInSpace(x, y, z - 1))
	{
		index = GetIndex(x, y, z - 1);
		result |= space[index];
	}

	return result;
}


}
