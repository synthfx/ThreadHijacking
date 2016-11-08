#pragma once

#include <Windows.h>

namespace RAII
{
	class Handle
	{
	public:
		// standard ctor
		Handle( HANDLE handle );

		// move ctor
		Handle( Handle&& rhs );

		// move assignment
		Handle& operator=( Handle&& rhs );

		~Handle( );

		// copy assignment
		Handle& operator=( const Handle& rhs ) = delete;

		// copy constructor
		Handle( const Handle& rhs ) = delete;

	public:
		operator bool( );
		HANDLE get( );

	private:
		HANDLE m_Handle;
	};
}