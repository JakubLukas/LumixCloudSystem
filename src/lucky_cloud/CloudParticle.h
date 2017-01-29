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

#pragma once

/*#pragma warning( push, 3 )
#pragma warning(disable:4786 4788)
#include <vector>
#include <algorithm>
#pragma warning( pop )
#pragma warning(disable:4786 4788)*/

#include "engine/vec.h"
#include <vector>

//-----------------------------------------------------------------------------
// Forward declarations
class VolumetricCloud;
class CParticlePool;
class CParticleEnumerator;


class CloudParticle
{
public:
	CParticlePool*	m_pParticlePool;
	unsigned int	m_i,m_j,m_k;
	unsigned int	m_uIndex;
	Lumix::Vec3		m_cScatteringColor;
	bool			m_bVisible;
public:
	CloudParticle();
	CloudParticle(unsigned int i, unsigned int j, unsigned int k, CParticlePool* pParticlePool);
	~CloudParticle();
	Lumix::Vec3* GetPosition();
	Lumix::Vec3* GetPositionFromLastBuffer();
	double GetViewDistance();
};


class CParticlePool
{
    friend class CParticleEnumerator;
    friend class CloudParticle;
public:
    CParticlePool();
    ~CParticlePool();
    void SortbyViewDistances(Lumix::Vec3 &vLookDir);
    void UpdateParticlePositions( double fTime );
    bool AddParticle(unsigned int i, unsigned int j, unsigned int k);
    bool Setup( VolumetricCloud *pVolumetricCloud, unsigned int uNumParticles );
    void Cleanup();

protected:
    VolumetricCloud *m_pVolumetricCloud;
    std::vector< CloudParticle* >	m_v_pCloudParticles[2];
	unsigned int			m_uNumParticles;
	Lumix::Vec3		*m_pvPositions[2];
    double			*m_pfViewDistances;
    double          m_PreTime[2];
    
public:
	Lumix::Vec3     m_vWindVelocity;
	unsigned int            m_iCurrentBuffer;

};


class CParticleEnumerator
{
public:
    CParticleEnumerator( CParticlePool *pParticlePool );
    ~CParticleEnumerator();
    CloudParticle* NextParticle();
    CloudParticle* NextParticleFromLastBuffer();
    void Reset();
private:
    CParticlePool *m_pParticlePool;
    unsigned int m_uIndex;
};
