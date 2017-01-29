#include "volumetric_cloud.h"
#include "Simulation.h"


static const float SAMPLE_LENGTH = 0.05f;
static const float OPTICAL_LENGTH_FACTOR = 1.5f; // determine the white and black contrast.more big more contrast.
static const float SCATTER_FACTOR = 0.6f; // According to the scattering model used, to have the intensity of scattering color less than the incident color,
// the maximum of SCATTER_FACTOR should be 2/3.
static const float MIN_DENSITY = 0.05f;
static const bool SSE4_ON = true;


const float square(float num)
{
	return num * num;
}


inline float OpticalLength(float fLineDensity)
{
	return OPTICAL_LENGTH_FACTOR * fLineDensity;
}


float Phase(Lumix::Vec3 vIn, Lumix::Vec3 vOut )
{
	vIn.normalize();
	vOut.normalize();
	return (1.0f + square(vIn.x * vOut.x + vIn.y * vOut.y + vIn.z * vOut.z)) * 3.0f / 4.0f;
}


VolumetricCloud::VolumetricCloud()
	: m_fLength(0.0f)
	, m_fWidth(0.0f)
	, m_fHigh(0.0f)
	, m_fCellSize(0.0f)
	, m_iLength(0)
	, m_iWidth(0)
	, m_iHigh(0)
	, m_fTimeA(0.0)
{
}


bool VolumetricCloud::Setup(const Environment& env, const CloudProperties& cloud)
{
	bool result = true;

	m_fLength = cloud.length;
	m_fWidth = cloud.width;
	m_fHigh = cloud.high;
	m_fCellSize = cloud.cellSize;
	m_iLength = (int)(m_fLength / m_fCellSize + 0.5);
	m_iWidth = (int)(m_fWidth / m_fCellSize + 0.5);
	m_iHigh = (int)(m_fHigh / m_fCellSize + 0.5);
	m_evolvingSpeed = cloud.evolvingSpeed;
	m_vCloudPos = cloud.cloudPos;

	m_windVelocity = env.windVelocity;

	result &= m_Simulator.Setup( m_iLength, m_iWidth, m_iHigh);
	result &= GenerateCloudParticles();

	m_ParticlePool.m_vWindVelocity = env.windVelocity;

	return result;
}


void VolumetricCloud::Cleanup()
{
	m_ParticlePool.Cleanup();
	m_Simulator.Cleanup();
}


VolumetricCloud::~VolumetricCloud()
{
}


bool VolumetricCloud::GenerateCloudParticles()
{
	bool result;
	result = m_ParticlePool.Setup(this, m_Simulator.GetNumCellInVolume());
	if (!result)
		return false;

	for (int i = 0; i < m_iLength; ++i)
	{
		for (int j = 0; j < m_iHigh; ++j)
		{
			for (int k = 0; k < m_iWidth; ++k)
			{
				if (m_Simulator.IsCellInVolume(i, j, k))
				{
					result = m_ParticlePool.AddParticle(i, j, k);
					if (!result)
						return false;
				}
			}
		}
	}

	return true;
}


void VolumetricCloud::CleanupCloudParticles()
{
	m_ParticlePool.Cleanup();
}


void VolumetricCloud::UpdateCloudPosition(float deltaTime)
{
    if (m_fTime == 0.0)  m_fTime = deltaTime; //first frame

    Lumix::Vec3 vDisplacement = m_windVelocity * (float)(deltaTime - m_fTime);
    m_vCloudPos += vDisplacement;
    m_ParticlePool.UpdateParticlePositions(deltaTime);

    //Change the cloud pos to let it loop moving
	//TODO: fuuuuuuuuuuuuj
    if (m_vCloudPos.x < -1000 || m_vCloudPos.x > 1000)
		m_vCloudPos.x *= -1;
	if(m_vCloudPos.y < -1000 || m_vCloudPos.y > 1000)
		m_vCloudPos.y *= -1;
	if(m_vCloudPos.z < -1000 || m_vCloudPos.z > 1000)
		m_vCloudPos.z *= -1;

    m_fTime = deltaTime;




		CParticleEnumerator Enumerator(&m_ParticlePool);
		CloudParticle *pCurParticle = Enumerator.NextParticle();
		while (pCurParticle)
		{
			Lumix::Vec3 curPartPos = Lumix::Vec3((float)pCurParticle->m_i, (float)pCurParticle->m_j, (float)pCurParticle->m_k);
			if (m_Simulator.IsPointInSpace(&curPartPos))
			{
				float fDensity = m_Simulator.GetPointDensity(&curPartPos);
				pCurParticle->m_cScatteringColor.x = fDensity;
				pCurParticle->m_cScatteringColor.y = fDensity;
				pCurParticle->m_cScatteringColor.z = fDensity;
			}

			pCurParticle = Enumerator.NextParticle();
		}





}


void VolumetricCloud::Update(float deltaTime)
{
	m_ParticlePool.m_iCurrentBuffer = 1 - m_ParticlePool.m_iCurrentBuffer;

	if (m_evolvingSpeed != 1.0) // if not pause evolving //TODO
	{
		float fAlpha = (float)(deltaTime - m_fTimeA ) / m_evolvingSpeed;
		m_Simulator.InterpolateDensitySpace( fAlpha);
		if( (deltaTime < m_fTimeA )|| (deltaTime > (m_fTimeA + m_evolvingSpeed)))
		{
			m_fTimeA = deltaTime;
		}
	}

	UpdateCloudPosition(deltaTime);
}
