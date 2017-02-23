#pragma once


#include "../simulation/utils.h"


namespace CldSim
{


struct Vec3
{
	float x;
	float y;
	float z;

	inline void Normalize();
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

	void SetCameraPosition(float x, float y, float z);
	void SetViewDirection(float x, float y, float z);


private:
	struct Box
	{
		Vec3 min;
		Vec3 max;
	};

	const uint viewSamples = 1;
	const uint lightSamples = 1;
	const float densityCutoff = 0.06f;
	const float densityFactor = 0.35f;
	const float attenuationFactor = 0.15f;
	const float colorMultiplier = 5.0f;

	uint m_width;
	uint m_height;
	uint m_length;
	Box m_box;
	Particle* m_particles = nullptr;
	float* m_densitySpace = nullptr;

	Vec3 m_cameraPosition{ 0.2f, 0.2f, 1.0f };
	Vec3 m_viewDirection{ 0.0f, 0.0f, 1.0f };
	Vec3 m_shadeColor{ 0.0f, 0.0f, 0.0f };
	Vec3 m_lightColor{ 1.0f, 1.0f, 1.0f };
	Vec3 m_sunPosition{ 1.0f, 1.0f, 1.0f };

	uint GetIndex(uint x, uint y, uint z) const;
	void CalcDensity(const bool* cloudSpace);
	inline float SingleDensity(uint x, uint y, uint z, const bool* cloudSpace, int S) const;
	inline void CalcSingleParticleColor(uint x, uint y, uint z);
};


}