/******************************************************************************************
*	CronoGames Game Engine																  *
*	Copyright � 2024 CronoGames <http://www.cronogames.net>								  *
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
#include "Support/exopch.h"
#include "Support/ExodusException.h"
#include "Support/WinInclude.h"
#include "Keyboard.h"
#include "Mouse.h"

namespace Exodus
{
	class Window
	{
	private:
		// singleton manages registration/cleanup of window class
		class WindowClass
		{
		public:
			static const char* GetName() noexcept;
			static LPCWSTR GetNameW() noexcept;
			static HINSTANCE GetInstance() noexcept;
		private:
			WindowClass() noexcept;
			~WindowClass();
			WindowClass( const WindowClass& ) = delete;
			WindowClass& operator=( const WindowClass& ) = delete;
			static constexpr const char* wndClassName = "Exodus Engine Window";
			static WindowClass wndClass;
			HINSTANCE hInst;
		};
	public:
		Window( int width, int height, const char* name, bool useAsSurface = false );
		~Window();
		Window( const Window& ) = delete;
		Window& operator=( const Window& ) = delete;
		void SetTitle( const std::string& title );
		static std::optional<int> ProcessMessages() noexcept;

		inline HWND GetHWND()
		{
			return _hWnd;
		}

		void SetFullscreen();
		void SetFullscreen( bool fullScreen );
		bool Resize();
		int32_t GetWidth();
		int32_t GetHeight();
		bool ShouldResize();
		void ResizeFinished();
	private:
		LPCWSTR ConvertToLPCWSTR( const char* charArray );
		static LRESULT CALLBACK HandleMsgSetup( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;
		static LRESULT CALLBACK HandleMsgThunk( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;
		LRESULT HandleMsg( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;

	public:
		Keyboard kbd;
		Mouse mouse;
	private:
		bool m_IMGuiInit = false;
		int _width;
		int _height;
		// Window rectangle (used to toggle fullscreen state).
		RECT _WindowRect;
		// By default, use windowed mode.
		// Can be toggled with the Alt+Enter or F11
		bool _Fullscreen = false;
		HWND _hWnd;
		std::vector<BYTE> rawBuffer;
		bool m_shouldResize = false;
	};

	Window* CreateCWindow( int width, int height, const char* name);
}
