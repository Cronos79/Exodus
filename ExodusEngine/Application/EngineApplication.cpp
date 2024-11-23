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
#include "EngineApplication.h"
#include "Debug/DXDebugLayer.h"
#include "D3D/DXContext.h"

namespace Exodus
{

	EngineApplication::EngineApplication( int width, int height, std::string title )
	{
		m_wnd = new Window( width, height, title.c_str() );
		m_timer = new ExodusTimer();
	}

	EngineApplication::~EngineApplication()
	{

	}

	bool EngineApplication::Run()
	{
		if (Init())
		{
			while (true)
			{
				// process all messages pending, but to not block for new messages
				if (const auto ecode = Window::ProcessMessages())
				{
					// if return optional has value, means we're quitting so return exit code
					Shutdown();
					return *ecode;
				}
				if (m_wnd->ShouldResize())
				{					
					DXContext::Get().Resize( m_wnd );
				}
				// execute the game logic
				const auto dt = m_timer->Mark() * m_speedFactor;
				
				DXContext::Get().BeginFrame();

				// DRAW
				HandleInput( dt );
				Update( dt );

				DXContext::Get().EndFrame();
			}
		}
		return -1;
	}


	bool EngineApplication::Init()
	{
		Exodus::DXDebugLayer::Get().Init();
		if (Exodus::DXContext::Get().Init(m_wnd))
		{
			return true;
		}
		DXContext::Get().Shutdown();
		return false;
	}

	void EngineApplication::Shutdown()
	{
		Exodus::DXContext::Get().Shutdown();
		Exodus::DXDebugLayer::Get().Shutdown();
	}

}