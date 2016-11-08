#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>
#include <fstream>
#include <codecvt>

namespace Utils
{
	uint32_t FindTargetThread( uint32_t targetProcessID );
	uint32_t GetTargetProcessID( const std::string& targetProcess );
	bool ResolveRelativePath( std::string& relativePath );
	uint64_t ReadFileFromStream( const std::string& filePath, byte* buffer = nullptr );
	std::wstring ToWideString( const std::string& multibyteString );
}