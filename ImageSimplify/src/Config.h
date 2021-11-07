#pragma once

class Config
{
	public:
		char* weightMapPath;
		char* contrastMapPath;
		int scaleFactor;
		float colorScaleFactor;
		float weightMult;
		float contrastMult;
		
		Config();
};