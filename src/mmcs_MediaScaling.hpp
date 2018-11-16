/*{REPLACEMEWITHLICENSE}*/
#pragma once

namespace mmcs {

void CenterFit(
	float iw,
	float ih,
	float vw,
	float vh,
	float & dw,
	float & dh,
	float & dx,
	float & dy
);

void ScaleSizePos(
	float scale,
	float & dw,
	float & dh,
	float & dx,
	float & dy
);

}
