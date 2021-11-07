#pragma once
#include "global.h"
#include "Vec2.h"

// Tracks probability distribution of a random variable via it's mean and variance
class Dist{
	public:
		float mean;
		float var;
		
		Dist();
		Dist(float mean,float var);

		// Assumes distributions are independant
		void abs();
		void add(Dist other);
		void sub(Dist other);
		void mult(float val);
		void div(float val);

		static Dist abs(Dist main);
		static Dist add(Dist main, Dist other);
		static Dist sub(Dist main, Dist other);
		static Dist mult(Dist main, float val);
		static Dist div(Dist main, float val);
		
		void applyTruncatedAdjustment(float minVa, float maxVal);
		Vec2<float> getPercentileRange(float percentile);
		
		static float smallPhi(float x);
		static float bigPhi(float x);
		static float invErf(float x);
		inline static float quantile(float p, float varSqrt);
};