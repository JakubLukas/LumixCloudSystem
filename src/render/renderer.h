#pragma once


#include "../simulation/utils.h"


namespace CldSim
{


struct Vec3
{
	float x;
	float y;
	float z;

	inline Vec3 Normalized();
	inline void operator +=(const Vec3& other);
	inline void operator -=(const Vec3& other);
};


class CloudRenderer
{
public:
	struct Particle
	{
		Vec3 position;
		Vec3 color;
	};

	~CloudRenderer();

	void Setup(uint width, uint height, uint length);
	void CalcParticleColors(const bool* cloudSpace);
	const Particle* GetParticles() const { return m_particles; }


private:
	const uint viewSamples = 128;
	const uint lightSamples = 64;
	const float densityCutoff = 0.06f;
	const float densityFactor = 0.35f;
	const float attenuationFactor = 0.15f;
	const float colorMultiplier = 5.0f;

	uint m_width;
	uint m_height;
	uint m_length;
	Particle* m_particles = nullptr;
	float* m_densitySpace = nullptr;

	uint GetIndex(uint x, uint y, uint z) const;
	void CalcDensity(const bool* cloudSpace);
	inline float SingleDensity(uint x, uint y, uint z, const bool* cloudSpace, int S) const;
	inline void CalcSingleParticleColor(uint x, uint y, uint z);
};


}