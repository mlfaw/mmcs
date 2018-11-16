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
	if (scale == 1.0f)
		return;
	float original_dw = dw, original_dh = dh;
	dw *= scale;
	dh *= scale;
	if (scale > 1.0f) {
		dx -= (dw - original_dw) / 2;
		dy -= (dh - original_dh) / 2;
	} else {
		dx += (original_dw - dw) / 2;
		dy += (original_dh - dh) / 2;
	}
}

}

