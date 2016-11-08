#include "Utils.hpp"

namespace Utils
{
	uint32_t FindTargetThread(uint32_t targetProcessID )
	{
		HANDLE snap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );

		if ( snap == INVALID_HANDLE_VALUE )
			return 0;

		THREADENTRY32 te32;
		te32.dwSize = sizeof( THREADENTRY32 );

		if ( Thread32First( snap, &te32 ) )
		{
			if ( te32.th32OwnerProcessID == targetProcessID )
				return te32.th32ThreadID;
		}

		while ( Thread32Next( snap, &te32 ) )
		{
			if ( te32.th32OwnerProcessID == targetProcessID )
				return te32.th32ThreadID;
		}

		return 0;
	}

	uint32_t GetTargetProcessID( const std::string& targetProcess )
	{
		std::wstring compareString = ToWideString( targetProcess );

		while ( true )
		{
			HANDLE snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

			if ( snap == INVALID_HANDLE_VALUE )
				continue;

			PROCESSENTRY32 pe32;
			pe32.dwSize = sizeof( PROCESSENTRY32 );

			if ( Process32First( snap, &pe32 ) )
			{
				if ( compareString.compare( pe32.szExeFile ) == 0 )
					return pe32.th32ProcessID;
			}

			while ( Process32Next( snap, &pe32 ) )
			{
				if ( compareString.compare( pe32.szExeFile ) == 0 )
					return pe32.th32ProcessID;
			}
		}
	}

	bool ResolveRelativePath( std::string& relativePath )
	{
		char current_path[ MAX_PATH ];
		
		if ( !GetModuleFileNameA( GetModuleHandle( nullptr ), current_path, MAX_PATH ) )
			return false;

		std::string cur_path( current_path );
		cur_path = cur_path.substr( 0, cur_path.rfind( '\\' ) + 1 );

		cur_path.append( relativePath );
		relativePath = cur_path;

		return true;
	}

	uint64_t ReadFileFromStream( const std::string& filePath, byte* buffer )
	{
		std::ifstream strm( filePath, std::ios::binary | std::ios::ate );
		std::streamsize size = strm.tellg( );

		if ( buffer == nullptr )
			return size;

		strm.seekg( 0, std::ios::beg );

		if ( strm.read( reinterpret_cast< char* >( buffer ), size ) )
			return 1;

		return 0;
	}

	std::wstring ToWideString( const std::string& multibyteString )
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring ret = converter.from_bytes( multibyteString );
		return ret;
	}
}