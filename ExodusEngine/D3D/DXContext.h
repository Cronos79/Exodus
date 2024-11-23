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
#pragma once
#include "Support/WinInclude.h"
#include "Support/ComPointer.h"
#include "Windows/Window.h"

namespace Exodus
{
	class DXContext
	{
	public:
		bool Init( Window* wnd );
		void Shutdown();

		void SignalAndWait();
		ID3D12GraphicsCommandList10* InitCommandList();
		void ExecuteCommandList();
		void Present();
		void ToggleVSync();
		void Flush();
		bool Resize( Window* wnd );

		inline ComPointer<IDXGIFactory7>& GetFactory()
		{
			return m_factory;
		}

		inline ComPointer<ID3D12Device14>& GetDevice()
		{
			return m_device;
		}

		inline ComPointer<ID3D12CommandQueue>& GetCommandQueue()
		{
			return m_cmdQueue;
		}

	private:	
		bool GetBuffers();
		void ReleaseBuffers();
		bool CreateSwapChain( Window* wnd );
		bool CheckTearingSupport();
		ComPointer<IDXGIAdapter4> GetAdapter( bool useWarp );
		ComPointer<ID3D12Device14> CreateDevice( ComPointer<IDXGIAdapter4> adapter );
	private:
		static constexpr int32_t m_bufferCount = 2;

		ComPointer<IDXGIFactory7> m_factory;
		ComPointer<ID3D12Device14> m_device;
		ComPointer<ID3D12CommandQueue> m_cmdQueue;

		ComPointer<ID3D12CommandAllocator> m_cmdAllocator;
		ComPointer<ID3D12GraphicsCommandList10> m_cmdList;

		ComPointer<IDXGISwapChain4> m_swapChain;
		ComPointer<ID3D12Resource2> m_buffers[m_bufferCount];

		ComPointer<ID3D12Fence1> m_fence;		
		HANDLE m_fenceEvent = nullptr;
		UINT64 m_fenceValue = 0;

		

		bool _TearingSupported;
		bool _rayTracingSupport;

		// Can be toggled with the V key.
		bool _VSync = true;

		// Singleton
	public:
		DXContext( const DXContext& ) = delete;
		DXContext& operator=( const DXContext& ) = delete;

		inline static DXContext& Get()
		{
			static DXContext instance;
			return instance;
		}
	private:
		DXContext() = default;
	};
}

