#pragma once

namespace msw {

// https://docs.microsoft.com/en-us/windows/desktop/medfound/saferelease
template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

}
