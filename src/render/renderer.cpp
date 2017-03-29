#include "renderer.h"

#include <math.h>


namespace CldSim
{


inline void Vec3::Normalize()
{
	float lenInv = 1.0f / sqrtf(x*x + y*y + z*z);
	x *= lenInv;
	y *= lenInv;
	z *= lenInv;
}


inline Vec3 Vec3::Normalized()
{
	float lenInv = 1.0f / sqrtf(x*x + y*y + z*z);
	return Vec3 {
		x * lenInv,
		y * lenInv,
		z * lenInv
	};
}


inline void Vec3::operator +=(const Vec3& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
}

inline void Vec3::operator -=(const Vec3& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
}

inline Vec3 operator *(float f, const Vec3& vec)
{
	return Vec3{
		f * vec.x,
		f * vec.y,
		f * vec.z
	};
}


struct Ray
{
	Vec3 origin;
	Vec3 direction;

	Ray(const Vec3& orig, const Vec3 dir)
		: origin(orig)
		, direction(dir)
	{ }
};


template <typename T>
inline static T min(T a, T b)
{
	return (a < b) ? a : b;
}

template <typename T>
inline static T max(T a, T b)
{
	return (a < b) ? b : a;
}

inline static float mix(float a, float b, float t)
{
	return a * (1.0f - t) + b * t;
}

inline static Color mix(Color a, Color b, float t)
{
	return Color{
		a.a * (1.0f - t) + b.a * t,
		a.r * (1.0f - t) + b.r * t,
		a.g * (1.0f - t) + b.g * t,
		a.b * (1.0f - t) + b.b * t
	};
}


inline static bool intersectRayBox(const Ray& r, const Vec3& boxMin, const Vec3& boxMax, float& t0, float& t1)
{
	Vec3 invR {
		1.0f / r.direction.x,
		1.0f / r.direction.y,
		1.0f / r.direction.z
	};

	Vec3 tbot {
		invR.x * (boxMin.x - r.origin.x),
		invR.y * (boxMin.y - r.origin.y),
		invR.z * (boxMin.z - r.origin.z)
	};

	Vec3 ttop {
		invR.x * (boxMax.x - r.origin.x),
		invR.y * (boxMax.y - r.origin.y),
		invR.z * (boxMax.z - r.origin.z)
	};

	Vec3 tmin{
		min(ttop.x, tbot.x),
		min(ttop.y, tbot.y),
		min(ttop.z, tbot.z)
	};

	Vec3 tmax{
		max(ttop.x, tbot.x),
		max(ttop.y, tbot.y),
		max(ttop.z, tbot.z)
	};

	float txy = max(tmin.x, tmin.y);
	float txz = max(tmin.x, tmin.z);

	t0 = max(txy, txz);

	txy = min(tmax.x, tmax.y);
	txz = min(tmax.x, tmax.z);

	t1 = min(txy, txz);

	return t0 <= t1;
}



CloudRenderer::~CloudRenderer()
{
	Clear();
}


void CloudRenderer::Clear()
{
	delete[] m_densitySpace;
	delete[] m_particles;
}


void CloudRenderer::Setup(uint width, uint height, uint length)
{
	m_width = width;
	m_height = height;
	m_length = length;

	int size = width * height * length;

	createArray(size, &m_densitySpace);
	m_particles = new Particle[size];

	uint index;
	for(uint x = 0; x < m_width; ++x)
	{
		for(uint y = 0; y < m_height; ++y)
		{
			for(uint z = 0; z < m_length; ++z)
			{
				index = GetIndex(x, y, z);
				m_particles[index].position = Vec3{
					(float)x,
					(float)y,
					(float)z
				};
			}
		}
	}

	m_box.min = Vec3{ 0.0f, 0.0f, 0.0f };
	m_box.max = Vec3{ (float)m_width, (float)m_height, (float)m_length };
}


void CloudRenderer::CalcParticleColors(const bool* cloudSpace)
{
	CalcDensity(cloudSpace);

	for(uint x = 0; x != m_width; ++x)
	{
		for(uint y = 0; y != m_height; ++y)
		{
			for(uint z = 0; z != m_length; ++z)
			{
				CalcSingleParticleColor(x, y, z);
			}
		}
	}
}


inline uint CloudRenderer::GetIndex(uint x, uint y, uint z) const
{
	return (x * m_height * m_length + y * m_length + z);
}


void CloudRenderer::CalcDensity(const bool* cloudSpace)
{
	static const int S = 6; // Blur matrix is size SxSxS //TODO: parametrizable
	for(uint x = 0; x != m_width; ++x)
	{
		for(uint y = 0; y != m_height; ++y)
		{
			for(uint z = 0; z != m_length; ++z)
			{
				m_densitySpace[GetIndex(x, y, z)]
					= SingleDensity(x, y, z, cloudSpace, S); // Do a box blur
			}
		}
	}
}


inline float CloudRenderer::SingleDensity(uint x, uint y, uint z, const bool* cloudSpace, int S) const
{
	// Go through kernel
	int halfS = (S - 1) / 2;
	float sum = 0;
	for(uint kX = x - halfS; kX <= x + halfS; ++kX)
		for(uint kY = y - halfS; kY <= y + halfS; ++kY)
			for(uint kZ = z - halfS; kZ <= z + halfS; ++kZ)
			{
				// Check if kernel isn't in the grid
				if(kX < 0 || kY < 0 || kZ < 0 || kX >= m_width || kY >= m_height || kZ >= m_length)
					continue; // Skip if it is

				sum += (float)cloudSpace[GetIndex(kX, kY, kZ)]; // w_i=1; box filter
			}
	return sum / (S * S * S);
}


inline void CloudRenderer::CalcSingleParticleColor(uint x, uint y, uint z)
{
	m_cameraPosition.Normalize();

	Ray viewRay = Ray(m_cameraPosition, m_viewDirection.Normalized());

	Color color { 1.0f, 0.0f, 0.0f, 0.0f };
	Vec3 pos = viewRay.origin;
	float tmin, tmax;
	intersectRayBox(viewRay, m_box.min, m_box.max, tmin, tmax);
	pos += tmax * viewRay.direction;
	float viewStepSize = (tmax - tmin) / m_viewSamples;

	Vec3 simSides{
		m_box.max.x - m_box.min.x,
		m_box.max.y - m_box.min.y,
		m_box.max.z - m_box.min.z
	};
	float maxDistance = sqrtf(simSides.x*simSides.x + simSides.y*simSides.y + simSides.z*simSides.z); // Length of a cube diagonal
	float lightStepSize = maxDistance / m_viewSamples;

	uint index = GetIndex(x, y, z);

	for(uint i = 0; i < m_viewSamples; ++i)
	{
		float cellDensity = m_densitySpace[index];
		if(cellDensity > densityCutoff)
		{

			cellDensity *= densityFactor;

			Ray lightRay = Ray(pos, m_sunPosition.Normalized()); // normalized really ?

			float attenuation = 1.0f;
			Vec3 lightPos = pos;

			// Calculate light attenuation
			for(uint j = 0; j < m_lightSamples; ++j)
			{
				uint lightIndex = GetIndex((uint)lightPos.x, (uint)lightPos.y, (uint)lightPos.z);
				// Closer texture reads contribute more
				attenuation *= 1.0f - m_densitySpace[lightIndex]
					* attenuationFactor
					* (1.0f - j / m_lightSamples);
				lightPos += lightStepSize * lightRay.direction;
			}

			// Add color depending on cell density and attenuation
			if(cellDensity > 0.001f)
			{
				color = mix(color, mix(m_shadeColor, m_sunColor, attenuation),
					cellDensity * colorMultiplier);
			}
		}

		pos -= viewStepSize * viewRay.direction;

	}

	m_particles[index].color = color;
	//outColor = vec4(color, 255);
	//vec4 debug = vec4(viewRay.direction, 1.0);
	//outColor = mix(outColor, debug, 0.0);
}


}