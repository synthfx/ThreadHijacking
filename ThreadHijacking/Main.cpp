#include "Utils.hpp"
#include "Handle.hpp"
#include "NT.hpp"

#include <iostream>
#include <Shlwapi.h>
#include <memory>

using tGetModuleHandleA = HMODULE( __stdcall* )( LPCSTR lpModuleName );

struct ShellcodeArgs
{
	uintptr_t imagebase, old_eip;
	tGetModuleHandleA getmodulehandle;
};

// 1. wildcard - pointer to shellcodeargs instance
// 2. wildcard - old eip
//byte shellc0de[ ] = { 0x60, 0x9C, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0x9D, 0x61 };

__declspec( ) void Shellcode( )
{
	__asm
	{
		pushad;
		pushfd;
		mov eax, 0xCCCCCCCC;
		mov ecx, 0xCCCCCCCC;
	}

	uintptr_t old_eip;
	ShellcodeArgs* args;

	_asm mov old_eip, ecx;
	_asm mov args, eax;

	__asm
	{
		popfd;
		popad;
		push old_eip;
		ret;
	}
}

__declspec( ) void ShellcodeEnd( )
{

}

int main( )
{
	// parsing information
	std::string process_name;
	std::string dll_path;

	std::cout << "Target Process: ";
	std::cin >> process_name;

	if ( process_name.rfind( ".exe" ) == std::string::npos )
		process_name.append( ".exe" );

	std::cout << "Source DLL: ";
	std::cin >> dll_path;

	if ( PathIsRelativeA( dll_path.c_str( ) ) )
		if ( !Utils::ResolveRelativePath( dll_path ) )
			return 0;


	// reading file into current process' memory
	uint64_t size = Utils::ReadFileFromStream( dll_path );

	if ( !( size > 0 ) )
		return 0;

	auto file_image = std::shared_ptr<byte>( new byte[ size ] );

	if ( !file_image )
		return 0;

	if ( !Utils::ReadFileFromStream( dll_path, file_image.get( ) ) )
		return 0;

	// opening target process
	uint32_t target_process_id = Utils::GetTargetProcessID( process_name );

	RAII::Handle target_process_handle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, target_process_id );

	if ( !target_process_handle )
		return 0;


	// allocate memory in target process and map dll file as image
	Internals::NT pe = reinterpret_cast< uintptr_t >( file_image.get( ) );

	auto dll_image = std::shared_ptr<byte>( reinterpret_cast< byte* >( VirtualAllocEx( target_process_handle.get( ), nullptr, pe.ImageSize( ), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE ) ), [&]( byte* memory ) {
		VirtualFreeEx( target_process_handle.get( ), memory, 0, MEM_RELEASE );
	} );

	if ( !dll_image.get( ) )
		return 0;

	if ( !pe.CopyHeaders( target_process_handle.get( ), reinterpret_cast< uintptr_t >( dll_image.get( ) ) ) )
		return 0;

	if ( !pe.CopySections( target_process_handle.get( ), reinterpret_cast< uintptr_t >( dll_image.get( ) ) ) )
		return 0;


	// find/open a thread to hijack
	uint32_t target_thread_id = 0;

	if ( !( target_thread_id = Utils::FindTargetThread( target_process_id ) ) )
		return 0;

	RAII::Handle target_thread_handle = OpenThread( THREAD_ALL_ACCESS, FALSE, target_thread_id );

	if ( !target_thread_handle )
		return 0;

	SuspendThread( target_thread_handle.get( ) );

	CONTEXT thread_context;
	thread_context.ContextFlags = CONTEXT_FULL;
	GetThreadContext( target_thread_handle.get( ), &thread_context );

	ShellcodeArgs args;
	args.getmodulehandle = GetModuleHandleA;
	args.imagebase = reinterpret_cast< uintptr_t >( dll_image.get( ) );
	args.old_eip = thread_context.Eip;

	auto args_memory = std::shared_ptr<byte>( reinterpret_cast< byte* >( VirtualAllocEx( target_process_handle.get( ), nullptr, sizeof( ShellcodeArgs ) , MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE ) ), [&]( byte* memory ) {
		VirtualFreeEx( target_process_handle.get( ), memory, 0, MEM_RELEASE );
	} );

	auto shellcode_memory = std::shared_ptr<byte>( reinterpret_cast< byte* >( VirtualAllocEx( target_process_handle.get( ), nullptr, (DWORD)ShellcodeEnd - (DWORD)Shellcode, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE ) ), [&]( byte* memory ) {
		VirtualFreeEx( target_process_handle.get( ), memory, 0, MEM_RELEASE );
	} );

	if ( !args_memory.get( ) )
		return 0;

	if ( !WriteProcessMemory( target_process_handle.get( ), args_memory.get( ), &args, sizeof( ShellcodeArgs ), nullptr ) )
		return 0;


}