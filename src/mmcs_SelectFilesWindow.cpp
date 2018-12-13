/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_SelectFilesWindow.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#else
// todo
#endif

namespace mmcs {

#ifdef _WIN32
std::vector<osstring> * SelectFilesWindow(bool foldersOnly, bool multiSelect)
{
// TODO: Decide if each OS impl should be kept in the same function definition...
	HRESULT hr;
	DWORD itemCount;
	DWORD dwFlags;
	PWSTR fileName = NULL;
	IFileOpenDialog * fileDialog = NULL;
	IShellItem * item = NULL;
	IShellItemArray * shellItems = NULL;
	std::vector<osstring> * files = NULL;

	GUID persistanceGuid;
	// Persists the previous program's folder location in dialogs.
	static PWSTR FileDialogPersistanceGUID =
		L"{F1CA2B12-F24C-4FCC-A840-23C610247504}";

	hr = IIDFromString(FileDialogPersistanceGUID, (IID *)&persistanceGuid);
	if (FAILED(hr)) return NULL;

	// TODO: Use the following around things that need to be ->Release()'d?
	// https://docs.microsoft.com/en-us/cpp/atl/reference/ccomptr-class?view=vs-2017
	// CComPtr<IShellItemArray> shellItems;
	// NOTE: CComPtr requires ATL (Active Template Library) so no...
	hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&fileDialog)
	);
	if (FAILED(hr)) return NULL;
	hr = fileDialog->SetClientGuid(persistanceGuid);
	if (FAILED(hr)) goto out;
	hr = fileDialog->GetOptions(&dwFlags);
	if (FAILED(hr)) goto out;
	dwFlags |= FOS_NOCHANGEDIR; // that's a no-no
	if (foldersOnly) dwFlags |= FOS_PICKFOLDERS;
	if (multiSelect) dwFlags |= FOS_ALLOWMULTISELECT;
	hr = fileDialog->SetOptions(dwFlags);
	if (FAILED(hr)) goto out;
	hr = fileDialog->Show(NULL);
	if (FAILED(hr)) goto out;
	hr = fileDialog->GetResults(&shellItems);
	if (FAILED(hr)) goto out;
	hr = shellItems->GetCount(&itemCount);
	if (FAILED(hr)) goto out;

	if (!itemCount) {
		hr = E_ABORT;
		goto out;
	}

	try {
		files = new std::vector<osstring>();
	} catch (...) {
		files = NULL;
		hr = E_ABORT;
		goto out;
	}

	for (DWORD i = 0; i < itemCount; ++i) {
		hr = shellItems->GetItemAt(i, &item);
		if (FAILED(hr)) goto out;
		hr = item->GetDisplayName(SIGDN_FILESYSPATH, &fileName);
		if (FAILED(hr)) goto out;

		try {
			files->push_back(osstring(fileName));
		} catch (...) {
			// an allocation error...
			// SUCCEEDED(hr) checks for >= 0, so let's have that fail
			hr = E_ABORT;
		}
		if (FAILED(hr)) goto out;

		CoTaskMemFree(fileName);
		fileName = NULL;
		item->Release();
		item = NULL;
	}

out:
	if (fileName)
		CoTaskMemFree(fileName);
	if (item)
		item->Release();
	if (shellItems)
		shellItems->Release();
	if (fileDialog)
		fileDialog->Release();
	if (!SUCCEEDED(hr) && files) {
		delete files;
		files = NULL;
	}
	return files;
}
#endif

}
