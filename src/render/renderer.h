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
inline Vec3 operator *(float f, const Vec3& vec);


struct Color
{
	float a;
	float r;
	float g;
	float b;
};


class CloudRenderer
{
public:
	struct Particle
	{
		Vec3 position;
		Color color;
	};

	~CloudRenderer();
	void Clear();

	void Setup(uint width, uint height, uint length);
	void CalcParticleColors(const bool* cloudSpace);
	const Particle* GetParticles() const { return m_particles; }

	void SetViewSamplesCount(const uint count) { m_viewSamples = count; }
	uint GetViewSamplesCount() const { return m_viewSamples; }

	void SetLightSamplesCount(const uint count) { m_lightSamples = count; }
	uint GetLightSamplesCount() const { return m_lightSamples; }

	void SetCameraPosition(const Vec3& pos) { m_cameraPosition = pos; }
	Vec3 GetCameraPosition() const { return m_cameraPosition; }

	void SetViewDirection(const Vec3& dir) { m_viewDirection = dir; }
	Vec3 GetViewDirection() const { return m_viewDirection; }

	void SetSunPosition(const Vec3& pos) { m_sunPosition = pos; }
	Vec3 GetSunPosition() const { return m_sunPosition; }

	void SetSunColor(const Color& color) { m_sunColor = color; }
	Color GetSunColor() const { return m_sunColor; }

	void SetShadeColor(const Color& color) { m_shadeColor = color; }
	Color GetShadeColor() const { return m_shadeColor; }


private:
	struct Box
	{
		Vec3 min;
		Vec3 max;
	};

	uint m_viewSamples = 1;
	uint m_lightSamples = 1;
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
	Vec3 m_sunPosition{ 1.0f, 1.0f, 1.0f };
	Color m_sunColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Color m_shadeColor{ 1.0f, 0.0f, 0.0f, 0.0f };

	uint GetIndex(uint x, uint y, uint z) const;
	void CalcDensity(const bool* cloudSpace);
	inline float SingleDensity(uint x, uint y, uint z, const bool* cloudSpace, int S) const;
	inline void CalcSingleParticleColor(uint x, uint y, uint z);
};


}