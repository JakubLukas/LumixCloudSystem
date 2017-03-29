#include "simulation.h"


namespace CldSim
{


Simulation::Simulation()
	: m_width(0)
	, m_height(0)
	, m_length(0)
	, m_frontBufIdx(1)
	, m_backBufIdx(0)
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
	m_backBufIdx = 0;
	m_frontBufIdx = 1;

	createArray(size, &m_hum[0]);
	createArray(size, &m_hum[1]);

	createArray(size, &m_act[0]);
	createArray(size, &m_act[1]);

	createArray(size, &m_cld[0]);
	createArray(size, &m_cld[1]);

	createArray(size, &m_ext[0]);
	createArray(size, &m_ext[1]);

	createArray(size, &m_extTimes);

	InitValues();

	return true;
}


void Simulation::InitValues()
{
	int size = m_width * m_height * m_length;

	for(int i = 0; i < size; ++i)
	{
		m_hum[m_frontBufIdx][i] = randFloat() < m_pHumInit;
		m_act[m_frontBufIdx][i] = m_hum[m_frontBufIdx][i] && (randFloat() < m_pActInit);
		m_ext[m_frontBufIdx][i] = 1;
		m_cld[m_frontBufIdx][i] = 0;
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
	for (uint x = 0; x < m_width; ++x)
	{
		for (uint y = 0; y < m_height; ++y)
		{
			for (uint z = 0; z < m_length; ++z)
			{
				Simulate(x, y, z, deltaTime);
			}
		}
	}

	m_frontBufIdx = m_backBufIdx;
	m_backBufIdx = 1 - m_backBufIdx;
}


void Simulation::Simulate(uint x, uint y, uint z, float deltaTime)
{
	int index = GetIndex(x, y, z);

	m_act[m_backBufIdx][index] =
		(!m_act[m_frontBufIdx][index]
			&& m_hum[m_frontBufIdx][index]
			&& CalcNeighborFunc(m_act[m_frontBufIdx], x, y, z))
		|| (randFloat() < m_pActUpdate);

	m_hum[m_backBufIdx][index] =
		(m_hum[m_frontBufIdx][index]
			&& !m_act[m_frontBufIdx][index])
		|| (randFloat() < m_pHumUpdate);

	m_ext[m_backBufIdx][index] =
		(!m_ext[m_frontBufIdx][index]
			&& m_cld[m_frontBufIdx][index]
			&& CalcNeighborFunc(m_ext[m_frontBufIdx], x, y, z))
		|| (randFloat() < m_pExtUpdate);

	bool newCld = !m_ext[m_frontBufIdx][index]
		&& (m_cld[m_frontBufIdx][index] || m_act[m_frontBufIdx][index]);

	//dont edit above !!!

	if(m_ext[m_frontBufIdx][index] && m_cld[m_frontBufIdx][index])//TODO: optimize
	{
		m_extTimes[index] += deltaTime;
		if(m_extTimes[index] > m_tExt)
		{
			m_extTimes[index] = 0.0f;
		}
		else
		{
			m_ext[m_backBufIdx][index] = true;
			newCld = true;
		}
	}

	m_cld[m_backBufIdx][index] = newCld;
}



uint Simulation::GetIndex(uint x, uint y, uint z) const
{
	return (x * m_height * m_length + y * m_length + z);
}


bool Simulation::IsCellInSpace(uint i, uint j, uint k) const
{
	return (i >= 0) && (i < m_width)
		&& (j >= 0) && (j < m_height)
		&& (k >= 0) && (k < m_length);
}


bool Simulation::CalcNeighborFunc(bool* space, uint x, uint y, uint z) const
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
