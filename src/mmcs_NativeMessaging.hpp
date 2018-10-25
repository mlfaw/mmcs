/*{REPLACEMEWITHLICENSE}*/
#pragma once

#ifndef MMCS_UWP
#include "mmcs_os.hpp"
#endif

namespace mmcs {

#ifndef MMCS_UWP
bool NativeMessaging_IsMode(int argc, oschar ** argv);
int NativeMessaging_Handler();
#endif

}
