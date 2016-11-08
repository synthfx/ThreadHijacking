#pragma once

#include <Windows.h>
#include <string>

#include "Handle.hpp"

namespace Internals
{
	class NT
	{
	public:
		NT( uintptr_t file_imagebase );

	public:
		size_t HeadersSize( );
		size_t SectionCount( );
		size_t ImageSize( );
		uintptr_t GetSectionByName( const std::string& sectionName );

		bool CopyHeaders( HANDLE handle, uintptr_t dllImage );
		bool CopySections( HANDLE handle, uintptr_t dllImage );

	private:
		uintptr_t m_ImageBase;
		IMAGE_DOS_HEADER* m_DosHeader;
		IMAGE_NT_HEADERS* m_NtHeaders;
	};
}