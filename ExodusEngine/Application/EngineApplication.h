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
#include <string>
#include "Windows/Window.h"
#include "ExodusTimer.h"

#include "entt.hpp" // https://github.com/skypjack/entt

namespace Exodus
{
	class EngineApplication
	{
	public:
		EngineApplication( int width, int height, std::string title);
		~EngineApplication();
		int32_t Run();
		virtual void HandleInput(float deltaTime) = 0;
		virtual void Update(float DeltaTime) = 0;
	private:
		bool Init();
		void Shutdown();
	protected:
		Window* m_wnd;
		ExodusTimer* m_timer;
		float m_speedFactor = 1.0f;
		entt::registry Entities;
	};
}
// To be defined in CLIENT
Exodus::EngineApplication* CreateEngineApp();