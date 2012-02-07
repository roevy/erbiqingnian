/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */
#include "D3D9RendererVertexBuffer.h"

#if defined(RENDERER_ENABLE_DIRECT3D9)

#include <RendererVertexBufferDesc.h>


static D3DVERTEXELEMENT9 buildVertexElement(WORD stream, WORD offset, D3DDECLTYPE type, BYTE method, BYTE usage, BYTE usageIndex)
{
	D3DVERTEXELEMENT9 element;
	element.Stream     = stream;
	element.Offset     = offset;
#if defined(RENDERER_WINDOWS)
	element.Type       = (BYTE)type;
#else
	element.Type       = type;
#endif
	element.Method     = method;
	element.Usage      = usage;
	element.UsageIndex = usageIndex;
	return element;
}

static D3DDECLTYPE getD3DType(RendererVertexBuffer::Format format)
{
	D3DDECLTYPE d3dType = D3DDECLTYPE_UNUSED;
	switch(format)
	{
		case RendererVertexBuffer::FORMAT_FLOAT1:  d3dType = D3DDECLTYPE_FLOAT1;   break;
		case RendererVertexBuffer::FORMAT_FLOAT2:  d3dType = D3DDECLTYPE_FLOAT2;   break;
		case RendererVertexBuffer::FORMAT_FLOAT3:  d3dType = D3DDECLTYPE_FLOAT3;   break;
		case RendererVertexBuffer::FORMAT_FLOAT4:  d3dType = D3DDECLTYPE_FLOAT4;   break;
		case RendererVertexBuffer::FORMAT_UBYTE4:  d3dType = D3DDECLTYPE_UBYTE4;   break;
		case RendererVertexBuffer::FORMAT_USHORT4: d3dType = D3DDECLTYPE_SHORT4;   break;
		case RendererVertexBuffer::FORMAT_COLOR:   d3dType = D3DDECLTYPE_D3DCOLOR; break;
	}
	ph_assert2(d3dType != D3DDECLTYPE_UNUSED, "Invalid Direct3D9 vertex type.");
	return d3dType;
}

static D3DDECLUSAGE getD3DUsage(RendererVertexBuffer::Semantic semantic, uint8 &usageIndex)
{
	D3DDECLUSAGE d3dUsage = D3DDECLUSAGE_FOG;
	usageIndex = 0;
	if(semantic >= RendererVertexBuffer::SEMANTIC_TEXCOORD0 && semantic <= RendererVertexBuffer::SEMANTIC_TEXCOORDMAX)
	{
		d3dUsage   = D3DDECLUSAGE_TEXCOORD;
		usageIndex = (uint8)(semantic - RendererVertexBuffer::SEMANTIC_TEXCOORD0);
	}
	else
	{
		switch(semantic)
		{
			case RendererVertexBuffer::SEMANTIC_POSITION:  d3dUsage = D3DDECLUSAGE_POSITION; break;
			case RendererVertexBuffer::SEMANTIC_NORMAL:    d3dUsage = D3DDECLUSAGE_NORMAL;   break;
			case RendererVertexBuffer::SEMANTIC_TANGENT:   d3dUsage = D3DDECLUSAGE_TANGENT;  break;
			case RendererVertexBuffer::SEMANTIC_COLOR:     d3dUsage = D3DDECLUSAGE_COLOR;    break;
			case RendererVertexBuffer::SEMANTIC_BONEINDEX:
				d3dUsage   = D3DDECLUSAGE_TEXCOORD;
				usageIndex = RENDERER_BONEINDEX_CHANNEL;
				break;
			case RendererVertexBuffer::SEMANTIC_BONEWEIGHT:
				d3dUsage   = D3DDECLUSAGE_TEXCOORD;
				usageIndex = RENDERER_BONEWEIGHT_CHANNEL;
				break;
		}
	}
	ph_assert2(d3dUsage != D3DDECLUSAGE_FOG, "Invalid Direct3D9 vertex usage.");
	return d3dUsage;
}

D3D9RendererVertexBuffer::D3D9RendererVertexBuffer(IDirect3DDevice9 &d3dDevice, const RendererVertexBufferDesc &desc, bool deferredUnlock) :
	RendererVertexBuffer(desc),
	m_d3dDevice(d3dDevice)
{
	m_d3dVertexBuffer = 0;
	m_deferredUnlock = deferredUnlock;
	
	m_usage      = 0;
	m_pool       = D3DPOOL_MANAGED;
	m_bufferSize = (UINT)(desc.maxVertices * m_stride);

	m_bufferWritten = false;
	
#if RENDERER_ENABLE_DYNAMIC_VB_POOLS
	if(desc.hint==RendererVertexBuffer::HINT_DYNAMIC)
	{
		m_usage = D3DUSAGE_DYNAMIC;
		m_pool  = D3DPOOL_DEFAULT;
	}
#endif
	
	onDeviceReset();
	
	if(m_d3dVertexBuffer)
	{
		m_maxVertices = desc.maxVertices;
	}
}

D3D9RendererVertexBuffer::~D3D9RendererVertexBuffer(void)
{
	if(m_d3dVertexBuffer)
	{
		m_d3dVertexBuffer->Release();
	}
}

void D3D9RendererVertexBuffer::addVertexElements(uint32 streamIndex, std::vector<D3DVERTEXELEMENT9> &vertexElements) const
{
	for(uint32 i=0; i<NUM_SEMANTICS; i++)
	{
		Semantic semantic = (Semantic)i;
		const SemanticDesc &sm = m_semanticDescs[semantic];
		if(sm.format < NUM_FORMATS)
		{
			uint8 d3dUsageIndex  = 0;
			D3DDECLUSAGE d3dUsage = getD3DUsage(semantic, d3dUsageIndex);
			vertexElements.push_back(buildVertexElement((WORD)streamIndex, (WORD)sm.offset, getD3DType(sm.format), D3DDECLMETHOD_DEFAULT, (BYTE)d3dUsage, d3dUsageIndex));
		}
	}
}

void *D3D9RendererVertexBuffer::lock(void)
{
	RENDERER_PERFZONE(D3D9RenderVBlock);
	void *lockedBuffer = 0;
	if(m_d3dVertexBuffer)
	{
		const uint32 bufferSize = m_maxVertices * m_stride;
		m_d3dVertexBuffer->Lock(0, (UINT)bufferSize, &lockedBuffer, 0);
		ph_assert2(lockedBuffer, "Failed to lock Direct3D9 Vertex Buffer.");
	}
	m_bufferWritten = true;
	return lockedBuffer;
}

void D3D9RendererVertexBuffer::unlock(void)
{
	RENDERER_PERFZONE(D3D9RenderVBunlock);
	if(m_d3dVertexBuffer)
	{
		m_d3dVertexBuffer->Unlock();
	}
}

void D3D9RendererVertexBuffer::bind(uint32 streamID, uint32 firstVertex)
{
	prepareForRender();
	if(m_d3dVertexBuffer)
	{
		m_d3dDevice.SetStreamSource((UINT)streamID, m_d3dVertexBuffer, firstVertex*m_stride, m_stride);
	}
}

void D3D9RendererVertexBuffer::unbind(uint32 streamID)
{
	m_d3dDevice.SetStreamSource((UINT)streamID, 0, 0, 0);
}

void D3D9RendererVertexBuffer::onDeviceLost(void)
{
	if(m_pool != D3DPOOL_MANAGED && m_d3dVertexBuffer)
	{
		m_d3dVertexBuffer->Release();
		m_d3dVertexBuffer = 0;
	}
}

void D3D9RendererVertexBuffer::onDeviceReset(void)
{
	if(!m_d3dVertexBuffer)
	{
		m_d3dDevice.CreateVertexBuffer(m_bufferSize, m_usage, 0, m_pool, &m_d3dVertexBuffer, 0);
		ph_assert2(m_d3dVertexBuffer, "Failed to create Direct3D9 Vertex Buffer.");
		m_bufferWritten = false;
	}
}

bool D3D9RendererVertexBuffer::checkBufferWritten(void)
{
	return m_bufferWritten;
}

#endif // #if defined(RENDERER_ENABLE_DIRECT3D9)
