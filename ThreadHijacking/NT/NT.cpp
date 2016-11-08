#include "NT.hpp"

namespace Internals
{
	NT::NT( uintptr_t file_imagebase )
	{
		m_ImageBase = file_imagebase;

		m_DosHeader = reinterpret_cast< IMAGE_DOS_HEADER* >( m_ImageBase );
		m_NtHeaders = reinterpret_cast< IMAGE_NT_HEADERS* >( m_ImageBase + m_DosHeader->e_lfanew );
	}

	size_t NT::HeadersSize( )
	{
		return m_NtHeaders->OptionalHeader.SizeOfHeaders;
	}

	size_t NT::SectionCount( )
	{
		return m_NtHeaders->FileHeader.NumberOfSections;
	}

	size_t NT::ImageSize( )
	{
		return m_NtHeaders->OptionalHeader.SizeOfImage;
	}

	uintptr_t NT::GetSectionByName( const std::string& sectionName )
	{
		auto cur_section = reinterpret_cast< IMAGE_SECTION_HEADER* >( m_NtHeaders + 1 );

		for ( size_t i = 0; i < SectionCount( ); i++ )
		{
			char name[ IMAGE_SIZEOF_SHORT_NAME + 1 ];
			ZeroMemory( name, IMAGE_SIZEOF_SHORT_NAME + 1 );
			memcpy( name, cur_section->Name, IMAGE_SIZEOF_SHORT_NAME );

			if ( sectionName.compare( reinterpret_cast< char* >( name ) ) == 0 )
				return m_ImageBase + cur_section->VirtualAddress;
		}

		return 0;
	}

	bool NT::CopyHeaders( HANDLE handle, uintptr_t dllImage )
	{
		return !!WriteProcessMemory( handle, reinterpret_cast< LPVOID >( dllImage ), reinterpret_cast< LPCVOID >( m_ImageBase ), HeadersSize( ), nullptr );
	}

	bool NT::CopySections( HANDLE handle, uintptr_t dllImage )
	{
		auto cur = reinterpret_cast< IMAGE_SECTION_HEADER* >( m_NtHeaders + 1 );

		for ( size_t i = 0; i < m_NtHeaders->FileHeader.NumberOfSections; i++, cur++ )
			if ( !WriteProcessMemory( handle, reinterpret_cast< LPVOID >( dllImage + cur->VirtualAddress ), reinterpret_cast< LPCVOID >( m_ImageBase + cur->PointerToRawData ), cur->SizeOfRawData, nullptr ) )
				return false;

		return true;
	}
}