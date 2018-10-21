/*{REPLACEMEWITHLICENSE}*/
#pragma once

namespace mmcs {

// Will block when spawning the process
void OpenUrlSync(const char * url);

// TODO: dup string & pass the process-spawning thread-pool?
//void OpenUrlAsync(const char * url);

}
