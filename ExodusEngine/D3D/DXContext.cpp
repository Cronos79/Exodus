/******************************************************************************************
*	CronoGames Game Engine																  *
*	Copyright © 2024 CronoGames <http://www.cronogames.net>								  *
*																						  *
*	This file is part of CronoGames Game Engine.										  *
*																						  *
*	CronoGames Game Engine is free software: you can redistribute it and/or modify		  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The CronoGames Game Engine is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The CronoGames Game Engine.  If not, see <http://www.gnu.org/licenses/>.   *
******************************************************************************************/
#include "exopch.h"
#include "DXContext.h"
#include "ExodusException.h"
#include "Shader.h"

namespace Exodus
{

	bool DXContext::Init(Window* wnd)
	{
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		if (FAILED( CreateDXGIFactory2( createFactoryFlags, IID_PPV_ARGS( &m_factory ) ) ))
		{
			return false;
		}

		_TearingSupported = CheckTearingSupport();
		ComPointer<IDXGIAdapter4> dxgiAdapter4 = GetAdapter( false );
		m_device = CreateDevice( dxgiAdapter4 );
		dxgiAdapter4.Release();
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 info = {};
		if (SUCCEEDED( m_device->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS5, &info, sizeof( info ) ) ))
		{
			switch (info.RaytracingTier)
			{
			case D3D12_RAYTRACING_TIER_1_0:
			case D3D12_RAYTRACING_TIER_1_1:
			{
				_rayTracingSupport = true;
			}break;
			case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
			{
				_rayTracingSupport = false;
			}break;
			default:
			{
				_rayTracingSupport = false;
			}break;
			}
		}
		if (m_device)
		{
			D3D12_COMMAND_QUEUE_DESC desc = {};
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc.NodeMask = 0;
			if (FAILED( m_device->CreateCommandQueue( &desc, IID_PPV_ARGS( &m_cmdQueue ) ) ))
			{
				return false;
			}

			if (FAILED( m_device->CreateFence( m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) ) ))
			{
				return false;				
			}

			m_fenceEvent = CreateEvent( nullptr, false, false, nullptr );
			if (!m_fenceEvent)
			{
				return false;
			}

			if (FAILED( m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_cmdAllocator ) ) ))
			{
				return false;
			}

			if (FAILED( m_device->CreateCommandList1( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS( &m_cmdList ) ) ))
			{
				return false;
			}		
		
			if (!CreateSwapChain( wnd ))
			{
				return false;
			}
		}

		InitMemoryBuffers();

		return true;
	}

	void DXContext::InitMemoryBuffers()
	{
		D3D12_HEAP_PROPERTIES hpUpload{};
		hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
		hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpUpload.CreationNodeMask = 0;
		hpUpload.VisibleNodeMask = 0;

		D3D12_HEAP_PROPERTIES hpDefault{};
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpDefault.CreationNodeMask = 0;
		hpDefault.VisibleNodeMask = 0;

		// == Vertex data ==		
		m_verticies[0] = { -1.f, -1.f };
		m_verticies[1] = { 0.f,  1.f };
		m_verticies[2] = { 1.f, -1.f };
		/*	{
				{ -1.f, -1.f },
				{ 0.f,  1.f },
				{ 1.f, -1.f },
			};	*/

		D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			{"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		D3D12_RESOURCE_DESC rd{};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rd.Width = 1024;
		rd.Height = 1;
		rd.DepthOrArraySize = 1;
		rd.MipLevels = 1;
		rd.Format = DXGI_FORMAT_UNKNOWN;
		rd.SampleDesc.Count = 1;
		rd.SampleDesc.Quality = 0;
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rd.Flags = D3D12_RESOURCE_FLAG_NONE;

		m_device->CreateCommittedResource( &hpUpload, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS( &m_uploadBuffer ) );
		m_device->CreateCommittedResource( &hpDefault, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS( &m_vertexBuffer ) );

		// Copy void* to cpu resource
		void* uploadBufferAddress;
		D3D12_RANGE uploadRange;
		uploadRange.Begin = 0;
		uploadRange.End = 1023;
		m_uploadBuffer->Map( 0, &uploadRange, &uploadBufferAddress );
		memcpy( uploadBufferAddress, m_verticies, sizeof( m_verticies ) );
		m_uploadBuffer->Unmap( 0, &uploadRange );
		// Copy cpu resource to gpu
		InitCommandList();
		m_cmdList->CopyBufferRegion( m_vertexBuffer, 0, m_uploadBuffer, 0, 1023 );
		ExecuteCommandList();

		Shader vertexShader( "VertexShader.cso" );
		Shader pixelShader( "PixelShader.cso" );

		// == Pipeline state ==
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{};
		gfxPsod.pRootSignature;
		gfxPsod.VS.pShaderBytecode = vertexShader.GetBuffer();
		gfxPsod.VS.BytecodeLength = vertexShader.GetSize();
		gfxPsod.PS.pShaderBytecode = pixelShader.GetBuffer();
		gfxPsod.PS.BytecodeLength = pixelShader.GetSize();
		gfxPsod.DS;
		gfxPsod.HS;
		gfxPsod.GS;
		gfxPsod.StreamOutput;
		gfxPsod.BlendState;
		gfxPsod.SampleMask;
		gfxPsod.RasterizerState;
		gfxPsod.DepthStencilState;
		gfxPsod.InputLayout.NumElements = _countof( vertexLayout );
		gfxPsod.InputLayout.pInputElementDescs = vertexLayout;
		gfxPsod.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		gfxPsod.PrimitiveTopologyType;
		gfxPsod.NumRenderTargets;
		gfxPsod.RTVFormats[8];
		gfxPsod.DSVFormat;
		gfxPsod.SampleDesc;
		gfxPsod.NodeMask;
		gfxPsod.CachedPSO;
		gfxPsod.Flags;

		m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vbv.SizeInBytes = sizeof( Vertex ) * _countof( m_verticies );
		m_vbv.StrideInBytes = sizeof( Vertex );
	}

	void DXContext::Shutdown()
	{
		Flush();
		ReleaseBuffers();
		if (m_uploadBuffer)
		{
			m_uploadBuffer.Release();
		}
		if (m_vertexBuffer)
		{
			m_vertexBuffer.Release();
		}
		if (m_rtvDescHeap)
		{
			m_rtvDescHeap.Release();
		}
		if (m_srvDescHeap)
		{
			m_srvDescHeap.Release();
		}
		if (m_factory)
		{
			m_factory.Release();
		}
		if (m_swapChain)
		{
			m_swapChain.Release();
		}
		if (m_cmdList)
		{
			m_cmdList.Release();
		}		
		if (m_cmdAllocator)
		{
			m_cmdAllocator.Release();
		}
		if (m_fenceEvent)
		{
			CloseHandle( m_fenceEvent );
		}
		if (m_fence)
		{
			m_fence.Release();
		}		
		if (m_cmdQueue)
		{
			m_cmdQueue.Release();
		}		
		if (m_device)
		{
			m_device.Release();
		}
		
	}

	void DXContext::SignalAndWait()
	{
		m_cmdQueue->Signal( m_fence, ++m_fenceValue );
		if (SUCCEEDED( m_fence->SetEventOnCompletion( m_fenceValue, m_fenceEvent ) ))
		{
			if (WaitForSingleObject( m_fenceEvent, 20000 ) != WAIT_OBJECT_0)
			{
				std::exit( -1 );
			}
		}	
		else
		{
			std::exit( -1 );
		}
	}

	ID3D12GraphicsCommandList10* DXContext::InitCommandList()
	{
		m_cmdAllocator->Reset();
		m_cmdList->Reset( m_cmdAllocator, nullptr );
		return m_cmdList;
	}

	void DXContext::ExecuteCommandList()
	{
		if (SUCCEEDED( m_cmdList->Close() ))
		{
			ID3D12CommandList* lists[] = { m_cmdList };
			m_cmdQueue->ExecuteCommandLists( 1, lists );
			SignalAndWait();
		}	
	}

	void DXContext::Present()
	{
		UINT syncInterval = _VSync ? 1 : 0;
		UINT presentFlags = _TearingSupported && !_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed( m_swapChain->Present( syncInterval, presentFlags ) );		
	}

	void DXContext::ToggleVSync()
	{
		_VSync = !_VSync;
	}

	void DXContext::Flush()
	{
		for (int32_t i = 0; i < m_bufferCount; i++)
		{
			SignalAndWait();
		}		
	}

	bool DXContext::Resize( Window* wnd )
	{		
		if (wnd->Resize())
		{
			ReleaseBuffers();
			Flush();
			m_swapChain->ResizeBuffers( m_bufferCount, wnd->GetWidth(), wnd->GetHeight(), DXGI_FORMAT_UNKNOWN, CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH );
			wnd->ResizeFinished();
			GetBuffers();
			return true;
		}
		return false;
	}

	void DXContext::BeginFrame()
	{
		InitCommandList();

		m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER barr;
		barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barr.Transition.pResource = m_buffers[m_currentBufferIndex];
		barr.Transition.Subresource = 0;
		barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_cmdList->ResourceBarrier( 1, &barr );

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void DXContext::BindInputAssembler()
	{
		m_cmdList->IASetVertexBuffers( 0, 1, &m_vbv );
		m_cmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	}

	void DXContext::Draw()
	{
		//m_cmdList->DrawInstanced( _countof( m_verticies ), 1, 0, 0 );
	}

	void DXContext::EndFrame()
	{
		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Rendering
		ImGui::Render();

		m_cmdList->ClearRenderTargetView( m_rtvHandles[m_currentBufferIndex], clearColor, 0, nullptr);
		m_cmdList->OMSetRenderTargets( 1, &m_rtvHandles[m_currentBufferIndex], FALSE, nullptr );
		m_cmdList->SetDescriptorHeaps( 1, &m_srvDescHeap );
		ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), m_cmdList );

		D3D12_RESOURCE_BARRIER barr;
		barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barr.Transition.pResource = m_buffers[m_currentBufferIndex];
		barr.Transition.Subresource = 0;
		barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		m_cmdList->ResourceBarrier( 1, &barr );	

		ExecuteCommandList();

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		Present();
	}

	bool DXContext::GetBuffers()
	{
		for (int32_t i = 0; i < m_bufferCount; ++i)
		{
			if (FAILED( m_swapChain->GetBuffer( i, IID_PPV_ARGS( &m_buffers[i] ) ) ))
			{
				return false;
			}
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;
			m_device->CreateRenderTargetView( m_buffers[i], &rtvDesc, m_rtvHandles[i] );
		}
		return true;
	}

	void DXContext::ReleaseBuffers()
	{
		for (int32_t i = 0; i < m_bufferCount; ++i)
		{
			m_buffers[i].Release();
		}

	}

	bool DXContext::CreateSwapChain( Window* wnd )
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = wnd->GetWidth();
		swapChainDesc.Height = wnd->GetHeight();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // #NOTE: Need to change this for hdr support
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = m_bufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// It is recommended to always allow tearing if tearing support is available.
		swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullDesc = {};
		swapChainFullDesc.Windowed = true;
		ComPointer<IDXGISwapChain1> swapChain1;
		if (FAILED( m_factory->CreateSwapChainForHwnd( m_cmdQueue, wnd->GetHWND(), &swapChainDesc, &swapChainFullDesc, nullptr, &swapChain1 ) ))
		{
			return false;
		}

		if (!swapChain1.QueryInterface( m_swapChain ))
		{
			return false;
		}

		//Create RTV heap
		D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc{};
		rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvDescHeapDesc.NumDescriptors = m_bufferCount;
		rtvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvDescHeapDesc.NodeMask = 0;
		if (FAILED( m_device->CreateDescriptorHeap( &rtvDescHeapDesc, IID_PPV_ARGS( &m_rtvDescHeap ) ) ))
		{
			return false;
		}

		{
			// Create handles to view
			auto firstHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
			auto handleIncrement = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
			for (int32_t i = 0; i < m_bufferCount; ++i)
			{
				m_rtvHandles[i] = firstHandle;
				m_rtvHandles[i].ptr += handleIncrement * i;
			}
		}

		//Create SRV heap
		D3D12_DESCRIPTOR_HEAP_DESC srvDescHeapDesc{};
		srvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvDescHeapDesc.NumDescriptors = 1;
		srvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvDescHeapDesc.NodeMask = 0;
		if (FAILED( m_device->CreateDescriptorHeap( &srvDescHeapDesc, IID_PPV_ARGS( &m_srvDescHeap ) ) ))
		{
			return false;
		}

		if (!GetBuffers())
		{
			return false;
		}

		ImGui_ImplDX12_Init( m_device, m_bufferCount,
			DXGI_FORMAT_R8G8B8A8_UNORM, m_srvDescHeap,
			m_srvDescHeap->GetCPUDescriptorHandleForHeapStart(),
			m_srvDescHeap->GetGPUDescriptorHandleForHeapStart() );

		return true;
	}

	bool DXContext::CheckTearingSupport()
	{
		BOOL allowTearing = FALSE;

		if (FAILED( m_factory->CheckFeatureSupport(
			DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allowTearing, sizeof( allowTearing ) ) ))
		{
			allowTearing = FALSE;
		}
		return allowTearing == TRUE;
	}

	ComPointer<IDXGIAdapter4> DXContext::GetAdapter( bool useWarp )
	{
		ComPointer<IDXGIAdapter1> dxgiAdapter1;
		ComPointer<IDXGIAdapter4> dxgiAdapter4;

		if (useWarp)
		{
			ThrowIfFailed( m_factory->EnumWarpAdapter( IID_PPV_ARGS( &dxgiAdapter1 ) ) );
			ThrowIfFailed( dxgiAdapter1->QueryInterface( &dxgiAdapter4 ) );
		}
		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; m_factory->EnumAdapters1( i, &dxgiAdapter1 ) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1( &dxgiAdapterDesc1 );

				// Check to see if the adapter can create a D3D12 device without actually 
				// creating it. The adapter with the largest dedicated video memory
				// is favored.
				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED( D3D12CreateDevice( dxgiAdapter1.Get(),
						D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr ) ) &&
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					ThrowIfFailed( dxgiAdapter1->QueryInterface( &dxgiAdapter4 ) );
				}
			}
		}

		if (dxgiAdapter1)
		{
			dxgiAdapter1->Release();
		}
		
		return dxgiAdapter4;
	}

	ComPointer<ID3D12Device14> DXContext::CreateDevice( ComPointer<IDXGIAdapter4> adapter )
	{
		ComPointer<ID3D12Device14> d3d12Device14;
		ThrowIfFailed( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &d3d12Device14 ) ) );
		// Enable debug messages in debug mode.
#if defined(_DEBUG)
		ComPointer<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED( d3d12Device14->QueryInterface( &pInfoQueue ) ))
		{
			pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE );
			pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, TRUE );
			pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, TRUE );
			// Suppress whole categories of messages
			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof( Severities );
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof( DenyIds );
			NewFilter.DenyList.pIDList = DenyIds;

			ThrowIfFailed( pInfoQueue->PushStorageFilter( &NewFilter ) );
		}
		pInfoQueue.Release();
#endif		
		return d3d12Device14;
	}

}
