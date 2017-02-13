#pragma once

#include "utils.h"


namespace CldSim
{


class Simulation
{
private:
	int m_width; //x
	int m_height; //y
	int m_length; //z

	uint m_elapsedSteps;
	uint m_actualIndex;

private:
	bool* m_hum[2];//TODO: optimize to bit field
	bool* m_act[2];
	bool* m_cld[2];
	bool* m_ext[2];
	float* m_extTimes;

	float m_pHum = 0.2f;
	float m_pAct = 0.2f;
	float m_tExt = 1.0f;


public:
	Simulation();
	~Simulation();

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetLength() const { return m_length; }

	inline int GetIndex(int x, int y, int z) const;

	bool Setup(uint width, uint height, uint length);
	void Clear();

	void Update(float deltaTime);

	const bool* GetCloudSpace() const { return m_cld[1 - m_actualIndex]; }
	const bool* GetActiveSpace() const { return m_act[1 - m_actualIndex]; }
	const bool* GetHumiditySpace() const { return m_hum[1 - m_actualIndex]; }

private:
	bool IsPointInSpace(const Vec3& point) const;
	bool IsCellInSpace(int x, int y, int z) const;

	inline void Simulate(int x, int y, int z);

	bool CalcNeighborFunc(bool* space, int x, int y, int z) const;
};


}
