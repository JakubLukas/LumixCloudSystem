//-------------------------------------------------------------------------------------
//
// Copyright 2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//
//--------------------------------------------------------------------------------------
// DXUT was adapted from the Microsoft DirectX SDK(November 2008)
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// The skybox is free downloaded from :
//   http://en.pudn.com/downloads119/sourcecode/others/detail508412_en.html
//-------------------------------------------------------------------------------------


#include "CloudParticle.h"
#include "volumetric_cloud.h"

#include "engine/lumix.h"


CloudParticle::CloudParticle()
{
	memset(this, 0, sizeof(CloudParticle));
}

CloudParticle::CloudParticle(unsigned int i, unsigned int j, unsigned int k, CParticlePool *pParticlePool)
	: m_i(i)
	, m_j(j)
	, m_k(k)
	, m_pParticlePool(pParticlePool)
	, m_bVisible(false)
{
}

CloudParticle::~CloudParticle()
{
}


Lumix::Vec3* CloudParticle::GetPosition()
{
	//Update thread, get data from current buffer
	ASSERT(m_pParticlePool && m_pParticlePool->m_pvPositions[m_pParticlePool->m_iCurrentBuffer]);
	return &(m_pParticlePool->m_pvPositions[m_pParticlePool->m_iCurrentBuffer][m_uIndex]);
}

Lumix::Vec3* CloudParticle::GetPositionFromLastBuffer()
{
	//Render thread, get data from last buffer
	ASSERT(m_pParticlePool && m_pParticlePool->m_pvPositions[1 - m_pParticlePool->m_iCurrentBuffer]);
	return &(m_pParticlePool->m_pvPositions[1 - m_pParticlePool->m_iCurrentBuffer][m_uIndex]);
}

double CloudParticle::GetViewDistance()
{
	ASSERT(m_pParticlePool && m_pParticlePool->m_pfViewDistances);
	return m_pParticlePool->m_pfViewDistances[m_uIndex];
}



CParticlePool::CParticlePool()
	: m_uNumParticles(0)
	, m_pfViewDistances(nullptr)
	, m_pVolumetricCloud(nullptr)
	, m_iCurrentBuffer(0)
{
	//these are for double buffer implementation
	m_pvPositions[0] = nullptr;
	m_pvPositions[1] = nullptr;
	m_PreTime[0] = 0.0;
	m_PreTime[1] = 0.0;
}

CParticlePool::~CParticlePool()
{
}


bool CompareViewDistance(CloudParticle* pElem1, CloudParticle* pElem2)
{
	return (pElem1->GetViewDistance() > pElem2->GetViewDistance());
}


void CParticlePool::SortbyViewDistances(Lumix::Vec3 &vLookDir)
{
	Lumix::Vec3 vToParticle;
    
    for( unsigned int i = 0; i < m_uNumParticles; i ++ )
    {
        vToParticle = m_pvPositions[m_iCurrentBuffer][i] - m_pVolumetricCloud->m_vCloudPos;
        m_pfViewDistances[i] = vLookDir.x * vToParticle.x + vLookDir.y * vToParticle.y + vLookDir.z * vToParticle.z;
    }

    //sort(m_v_pCloudParticles[m_iCurrentBuffer].begin(), m_v_pCloudParticles[m_iCurrentBuffer].end(), CompareViewDistance);

}

void CParticlePool::UpdateParticlePositions( double fTime )
{
    if ((m_PreTime[0] == 0.0) || (m_PreTime[1] == 0.0))
    {
        //The first frame, init two time members
        m_PreTime[0] = fTime;
        m_PreTime[1] = fTime;
    }
    float fElapsedTime = (float)(fTime - m_PreTime[m_iCurrentBuffer]);
	Lumix::Vec3 vDisplacement = m_vWindVelocity * fElapsedTime;
    for( unsigned int i = 0; i < m_uNumParticles; i ++ )
    {
        m_pvPositions[m_iCurrentBuffer][i] += vDisplacement;
        //Change the cloud pos to make it loop moving
        if (m_pvPositions[m_iCurrentBuffer][i].z < -1000) m_pvPositions[m_iCurrentBuffer][i].z += 1500;
    }
    m_PreTime[m_iCurrentBuffer] = fTime;
}

bool CParticlePool::Setup( VolumetricCloud *pVolumetricCloud, unsigned int uNumParticles )
{
    m_pVolumetricCloud = pVolumetricCloud;

    m_pfViewDistances = new double[uNumParticles];
    if( !m_pfViewDistances ) return false;

    m_pvPositions[0] = new Lumix::Vec3[uNumParticles];
    if( !m_pvPositions[0] ) return false;

    m_pvPositions[1] = new Lumix::Vec3[uNumParticles];
    if( !m_pvPositions[1] ) return false;
    
    m_uNumParticles = uNumParticles;
    return true;
}

void CParticlePool::Cleanup()
{
    if( m_pfViewDistances )
        delete[] m_pfViewDistances;

    //cleanup double buffer data
    if( m_pvPositions[0] )
        delete[] m_pvPositions[0];
    if( m_pvPositions[1] )
        delete[] m_pvPositions[1];

    std::vector< CloudParticle* >::iterator itCurCP, itEndCP = m_v_pCloudParticles[0].end();
    for( itCurCP = m_v_pCloudParticles[0].begin(); itCurCP != itEndCP; ++ itCurCP )
    {
        delete ( * itCurCP );
    }
    m_v_pCloudParticles[0].clear();
		std::vector< CloudParticle* >::iterator itCurCP2, itEndCP2 = m_v_pCloudParticles[1].end();
    for( itCurCP2 = m_v_pCloudParticles[1].begin(); itCurCP2 != itEndCP2; ++ itCurCP2 )
    {
        delete ( * itCurCP2 );
    }
    m_v_pCloudParticles[1].clear();
}

bool CParticlePool::AddParticle(unsigned int i, unsigned int j, unsigned int k)
{
    unsigned int index = (unsigned int)m_v_pCloudParticles[0].size();
    if( index >= m_uNumParticles )
		return false;

    CloudParticle *pPreParticleBuffer = new CloudParticle(i,j,k,this);
    if( pPreParticleBuffer == NULL )
		return false;

    pPreParticleBuffer->m_uIndex = index;
    m_pvPositions[0][index] = m_pVolumetricCloud->m_vCloudPos + m_pVolumetricCloud->m_fCellSize * Lumix::Vec3( (float)i, (float)j, (float)k );

    //add to double buffer 1
    m_v_pCloudParticles[0].push_back(pPreParticleBuffer);

    CloudParticle *pCurrentParticleBuffer = new CloudParticle(i,j,k,this);
    if( pCurrentParticleBuffer == NULL ) return false;

    pCurrentParticleBuffer->m_uIndex = index;
    m_pvPositions[1][index] = m_pVolumetricCloud->m_vCloudPos + m_pVolumetricCloud->m_fCellSize * Lumix::Vec3( (float)i, (float)j, (float)k );

    //add to double buffer 2
    m_v_pCloudParticles[1].push_back(pCurrentParticleBuffer);

    return true;
}


CParticleEnumerator::CParticleEnumerator( CParticlePool *pParticlePool )
{
    m_pParticlePool = pParticlePool;
    m_uIndex = 0;
}

CParticleEnumerator::~CParticleEnumerator( )
{
}


CloudParticle* CParticleEnumerator::NextParticle()
{
    //Update thread, get data from current buffer
    if( m_uIndex >= m_pParticlePool->m_uNumParticles )
        return NULL;
    else
        return m_pParticlePool->m_v_pCloudParticles[m_pParticlePool->m_iCurrentBuffer][m_uIndex++];
}

CloudParticle* CParticleEnumerator::NextParticleFromLastBuffer()
{
    //Render thread, get data from last buffer
    if( m_uIndex >= m_pParticlePool->m_uNumParticles )
        return NULL;
    else
        return m_pParticlePool->m_v_pCloudParticles[1-m_pParticlePool->m_iCurrentBuffer][m_uIndex++];
}

void CParticleEnumerator::Reset()
{
    m_uIndex = 0;
}
