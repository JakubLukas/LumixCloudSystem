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

	float m_pHumInit = 0.2f;
	float m_pActInit = 0.2f;

	float m_tExt = 2.0f;

	float m_pHumUpdate = 0.01f;
	float m_pActUpdate = 0.1f;
	float m_pExtUpdate = 0.995f;


public:
	Simulation();
	~Simulation();

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetLength() const { return m_length; }

	inline int GetIndex(int x, int y, int z) const;

	bool Setup(uint width, uint height, uint length);
	inline void InitValues();
	void Clear();
	void Restart();

	void Update(float deltaTime);

	const bool* GetCloudSpace() const { return m_cld[1 - m_actualIndex]; }
	const bool* GetActiveSpace() const { return m_act[1 - m_actualIndex]; }
	const bool* GetHumiditySpace() const { return m_hum[1 - m_actualIndex]; }

	float GetHumidityProbability() const { return m_pHumUpdate; }
	void SetHumidityProbability(float value) { m_pHumUpdate = value; }

	float GetActiveProbability() const { return m_pActUpdate; }
	void SetActiveProbability(float value) { m_pActUpdate = value; }

	float GetExtensionProbability() const { return m_pExtUpdate; }
	void SetExtensionProbability(float value) { m_pExtUpdate = value; }

	float GetExtinctionTime() const { return m_tExt; }
	void SetExtinctionTime(float value) { m_tExt = value; }

private:
	bool IsCellInSpace(int x, int y, int z) const;

	inline void Simulate(int x, int y, int z, float deltaTime);

	bool CalcNeighborFunc(bool* space, int x, int y, int z) const;
};


}
