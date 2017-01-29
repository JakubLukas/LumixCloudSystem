#pragma once

#include "Simulation.h"
#include "CloudParticle.h"

#include "engine/vec.h"


struct Environment
{
	Lumix::Vec3 windVelocity;
};

struct CloudProperties
{
	float length;
	float width;
	float high;
	float cellSize;
	float evolvingSpeed;
	Lumix::Vec3 cloudPos;
};


class VolumetricCloud
{
private:
	float m_evolvingSpeed;
	Lumix::Vec3 m_windVelocity;

	CSimulationSpace m_Simulator;

public:
	float m_fLength;
	float m_fWidth;
	float m_fHigh;

	float m_fCellSize;

	int m_iLength;
	int m_iWidth;
	int m_iHigh;

	Lumix::Vec3 m_vCloudPos;

	float m_fTime;
	float m_fTimeA;

	CParticlePool m_ParticlePool;

public:
	VolumetricCloud();
	~VolumetricCloud();
	bool Setup(const Environment& env, const CloudProperties& cloud);
	void Cleanup();
	void Update(float deltaTime);

	bool GenerateCloudParticles();
	void CleanupCloudParticles();
	void UpdateCloudPosition(float deltaTime);


	float GetEvolvingSpeed() const { return m_evolvingSpeed; }
	void SetEvolvingSpeed(float speed) { m_evolvingSpeed = speed; }

	Lumix::Vec3 GetWindVelocity() const { return m_windVelocity; }
	void SetWindVelocity(Lumix::Vec3 windVelocity)
	{
		m_windVelocity = windVelocity;
		m_ParticlePool.m_vWindVelocity = windVelocity;
	}
};
