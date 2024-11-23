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
#include "exopch.h"
#include "WinInclude.h"
#include <exception>


namespace Exodus
{
	class ExodusException : public std::exception
	{
	public:
		ExodusException( int line, const char* file ) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;
		int GetLine() const noexcept;
		const std::string& GetFile() const noexcept;
		std::string GetOriginString() const noexcept;
	private:
		int line;
		std::string file;
	protected:
		mutable std::string whatBuffer;
	};
	class HrException : public ExodusException
	{
	public:
		HrException( int line, const char* file, HRESULT hr ) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
		static std::string TranslateErrorCode( HRESULT hr ) noexcept;
	private:
		HRESULT hr;
	};
	class NoGfxException : public ExodusException
	{
	public:
		using ExodusException::ExodusException;
		const char* GetType() const noexcept override;
	};	
}

#define EHWND_EXCEPT( hr ) Exodus::HrException( __LINE__,__FILE__,(hr) )
#define EHWND_LAST_EXCEPT() Exodus::HrException( __LINE__,__FILE__,GetLastError() )
#define EHWND_NOGFX_EXCEPT() Exodus::NoGfxException( __LINE__,__FILE__ )

namespace Exodus
{
	inline void ThrowIfFailed( HRESULT hr )
	{
		if (FAILED( hr ))
		{
			throw EHWND_EXCEPT( hr );
		}
	}
}