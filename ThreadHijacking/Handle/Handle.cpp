#include "Handle.hpp"

namespace RAII
{
	Handle::Handle( HANDLE handle )
	{
		m_Handle = handle;
	}

	Handle::Handle( Handle&& rhs )
	{
		m_Handle = rhs.m_Handle;
		rhs.m_Handle = INVALID_HANDLE_VALUE;
	}

	Handle& Handle::operator=( Handle&& rhs )
	{
		m_Handle = rhs.m_Handle;
		rhs.m_Handle = INVALID_HANDLE_VALUE;

		return *this;
	}

	Handle::~Handle( )
	{
		if ( m_Handle != INVALID_HANDLE_VALUE )
			CloseHandle( m_Handle );
	}

	Handle::operator bool( )
	{
		return ( m_Handle != INVALID_HANDLE_VALUE && m_Handle != NULL );
	}

	HANDLE Handle::get( )
	{
		return m_Handle;
	}
}