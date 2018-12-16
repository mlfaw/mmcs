/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_NativeMessaging.hpp"
#include <string.h> // strrchr(), strcmp()

namespace mmcs {
namespace NativeMessaging {

static const oschar * allowedExtensions[] = {
	_OS("chrome-extension://[extensionID]"),
	_OS("ping_pong@example.org"),
};
static const int allowedExtensionsCount =
	sizeof(allowedExtensions) / sizeof(allowedExtensions[0]);

static bool IsMode(int argc, oschar ** argv)
{
	// Firefox:
	//   mmcs C:\path\to\native-messaging\mmcs.json ping_pong@example.org
	// Chrome:
	//   mmcs chrome-extension://[extensionID]
	// Chrome Windows:
	//   mmcs chrome-extension://[extensionID] --parent-window=1234

	const oschar * extension_id;
	const oschar * manifest_ext;

	if (argc < 2)
		return false;

	// Chrome's extension_id location.
	extension_id = argv[1];

	// Check for the Firefox extension manifest.json file...
	manifest_ext = osstrrchr(argv[1], _OS('.'));
	if (manifest_ext && (osstrcmp(manifest_ext, _OS(".json")) == 0)) {
		// Yes, Firefox triggered us so the extension_id is argv[2].
		if (argc < 3)
			return false;
		extension_id = argv[2];
	}

	for (int i = 0; i < allowedExtensionsCount; ++i) {
		if (osstrcmp(allowedExtensions[i], extension_id) == 0)
			return true;
	}

	return false;
}

bool Handler(int argc, oschar ** argv)
{
	if (!IsMode(argc, argv))
		return false;
	return true;
}

}
}
