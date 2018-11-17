/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_NativeMessaging.hpp"
#include "mmcs_StringHelpers.hpp"

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

	if (argc < 2)
		return false;

	// Check for Chrome...
	if (osStringInArray(argv[1], allowedExtensions, allowedExtensionsCount))
		return true;

	if (argc < 3)
		return false;

	// Now for Firefox...
	// The FF manifest...
	if (!osStringEndsWithExtension(argv[1], _OS(".json")))
		return false;

	// The FF extension...
	if (!osStringInArray(argv[2], allowedExtensions, allowedExtensionsCount))
		return false;

	return true;
}

bool Handler(int argc, oschar ** argv)
{
	if (!IsMode(argc, argv))
		return false;
	return true;
}

}
}
