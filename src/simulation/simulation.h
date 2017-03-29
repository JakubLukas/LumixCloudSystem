#pragma once

#include "utils.h"


namespace CldSim
{


class Simulation
{
public:
	Simulation();
	~Simulation();

	uint GetWidth() const { return m_width; }
	uint GetHeight() const { return m_height; }
	uint GetLength() const { return m_length; }

	inline uint GetIndex(uint x, uint y, uint z) const;

	bool Setup(uint width, uint height, uint length);
	inline void InitValues();
	void Clear();
	void Restart();

	void Update(float deltaTime);

	const bool* GetCloudSpace() const { return m_cld[m_frontBufIdx]; }
	const bool* GetActiveSpace() const { return m_act[m_frontBufIdx]; }
	const bool* GetHumiditySpace() const { return m_hum[m_frontBufIdx]; }
	const float* GetExtinctionTimes() const { return m_extTimes; }

	float GetHumidityProbability() const { return m_pHumUpdate; }
	void SetHumidityProbability(float value) { m_pHumUpdate = value; }

	float GetActiveProbability() const { return m_pActUpdate; }
	void SetActiveProbability(float value) { m_pActUpdate = value; }

	float GetExtinctionProbability() const { return m_pExtUpdate; }
	void SetExtinctionProbability(float value) { m_pExtUpdate = value; }

	float GetExtinctionTime() const { return m_tExt; }
	void SetExtinctionTime(float value) { m_tExt = value; }


private:
	uint m_width; //x
	uint m_height; //y
	uint m_length; //z

	uint m_elapsedSteps;
	uint m_frontBufIdx;
	uint m_backBufIdx;


	bool* m_hum[2];//TODO: optimize to bit field?
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


	bool IsCellInSpace(uint x, uint y, uint z) const;

	inline void Simulate(uint x, uint y, uint z, float deltaTime);

	bool CalcNeighborFunc(bool* space, uint x, uint y, uint z) const;
};


}
