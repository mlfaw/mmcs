/*{REPLACEMEWITHLICENSE}*/
#include "win32_misc.hpp"
#include <Windows.h>
#include <winnt.h> // NtCurrentTeb()
#include <winternl.h> // PTEB, PPEB

namespace win32 {

const wchar_t * GetExePath()
{
    return NtCurrentTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName.Buffer;
}

}
