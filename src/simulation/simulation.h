#pragma once

#include "utils.h"


class Simulation
{
private:
	int m_width; //x
	int m_height; //y
	int m_length; //z

	uint m_elapsedSteps;
	uint m_actualIndex;

private:
	float* m_densitySpace[2];

public:
	Simulation();
	~Simulation();

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetLength() const { return m_length; }

	int GetIndex(int x, int y, int z) const;

	bool Setup(uint width, uint height, uint length);
	void Clear();

	void Update(float deltaTime);

	const float* GetDensitySpace() const { return m_densitySpace[1 - m_actualIndex]; }

private:
	bool IsPointInSpace(const Vec3& point) const;
	bool IsCellInSpace(int x, int y, int z) const;

	void Simulate(int x, int y, int z);

	template<typename Type>
	int GetNeighborCount(Type* space, int x, int y, int z, Type valueGreaterThan) const;
};
