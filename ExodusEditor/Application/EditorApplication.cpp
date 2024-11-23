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
#include "EditorApplication.h"

namespace Exodus
{

	EditorApplication::EditorApplication( int width, int height, std::string title )
		: EngineApplication(width, height, title)
	{

	}

	EditorApplication::~EditorApplication()
	{

	}

	void EditorApplication::HandleInput( float deltaTime )
	{
		if(m_wnd->kbd.KeyIsPressed( VK_F11 ))
		{
			m_wnd->SetFullscreen();
		}
	}

	void EditorApplication::Update( float DeltaTime )
	{
		
	}

}

Exodus::EngineApplication* CreateEngineApp()
{
	return new Exodus::EditorApplication( 1920, 1080, "CronoEditor" );
}