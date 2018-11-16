/*{REPLACEMEWITHLICENSE}*/
#include "mmcs_MediaScaling.hpp"

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
)
{
	float scalew = vw / iw;
	float scaleh = vh / ih;
	float scale = (scalew < scaleh) ? scalew : scaleh;

	dw = iw * scale;
	dh = ih * scale;
	dx = (dw < vw) ? ((vw / 2) - (dw / 2)) : 0;
	dy = (dh < vh) ? ((vh / 2) - (dh / 2)) : 0;
}

void ScaleSizePos(
	float scale,
	float & dw,
	float & dh,
	float & dx,
	float & dy
)
{

}

}

