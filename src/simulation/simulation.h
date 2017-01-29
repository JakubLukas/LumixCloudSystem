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

	bool Setup(uint width, uint height, uint length);
	void Cleanup();

	void Update(float deltaTime);

private:
};
