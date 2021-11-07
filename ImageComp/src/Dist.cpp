#include "Dist.h"
#include <cmath>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#include <algorithm>

Dist::Dist(){
	mean = 0;
	var = 0;
}

Dist::Dist(float mean, float var){
	this->mean = mean;
	this->var = var;
}

void Dist::abs() {
	*this = Dist::abs(*this);
}

void Dist::add(Dist other) {
	*this = Dist::add(*this, other);
}

void Dist::sub(Dist other) {
	*this = Dist::sub(*this, other);
}

void Dist::mult(float val) {
	*this = Dist::mult(*this, val);
}

void Dist::div(float val) {
	*this = Dist::div(*this, val);
}

Dist Dist::abs(Dist main) {
	return Dist(main.mean > 0 ? main.mean : -main.mean,
				main.var);
}

Dist Dist::add(Dist main, Dist other) {
	return Dist(main.mean + other.mean,
				main.var + other.var);
}

Dist Dist::sub(Dist main, Dist other){
	return Dist(main.mean - other.mean,
				main.var + other.var);
}

Dist Dist::mult(Dist main, float val) {
	return Dist(main.mean * val,
				main.var * (val * val));
}

Dist Dist::div(Dist main, float val) {
	if (val == 0) {
		printf("Err: Dist division by 0.");
		exit(1);
	}
	return Dist(main.mean / val,
		main.var / (val * val));
}

/*
 Using calculations for mean and variance as given here:
	https://en.wikipedia.org/wiki/Truncated_normal_distribution
 I don't understand the reasoning behind the numbers but I've
 experimentally verified that the adjustments result in more accurate results.
*/
void Dist::applyTruncatedAdjustment(float minVal, float maxVal){
	if(var/(maxVal-minVal) < 0.1){
		/* Variance is so low that adjustment wouldn't do much and numeric errors
			might result in less accurate results
			Also covers var=0 case */
		return;
	}
	
	float varSqrt = sqrt(var);
	float alpha = (minVal-mean)/varSqrt;
	float beta = (maxVal-mean)/varSqrt;
	float zVal = Dist::bigPhi(beta) - Dist::bigPhi(alpha);
	
	// Calculate adjusted mean
	float adjMean = this->mean + ((Dist::smallPhi(alpha) - Dist::smallPhi(beta))*varSqrt) / zVal;
	
	// Calculate adjusted variance
	float adjVar = 1.0f + (alpha*Dist::smallPhi(alpha) - beta*Dist::smallPhi(beta))/zVal;
	float tmp = (Dist::smallPhi(alpha) - Dist::smallPhi(beta))/zVal;
	adjVar -= tmp*tmp;
	adjVar *= this->var;
	
	this->mean = adjMean;
	this->var = adjVar;
}

/* Ex:
	0.8 percentile -> range 80% of values fall into
 */
Vec2<float> Dist::getPercentileRange(float percentile){
	if(var < 0.0001){
		// Variance low enough to be considered 0
		return Vec2<float>(mean,mean);
	}
	
	// dist is always <=0 (assuming 0<= percentile <= 1)
	float dist = Dist::quantile((1.0f - percentile)/2.0f,sqrt(var));
	assert(dist <= 0);
	return Vec2<float>(mean+dist,mean-dist);
}

float Dist::smallPhi(float x){
	return (1.0f/sqrt((float)M_PI*2.0f))*exp(-0.5f*x*x);
}

float Dist::bigPhi(float x){
	return 0.5f*(1.0f + erf(x/sqrt(2.0f)));
}

// https://stackoverflow.com/a/40260471
float Dist::invErf(float x){
   float tt1, tt2, lnx, sgn;
   sgn = (x < 0) ? -1.0f : 1.0f;

   x = (1 - x)*(1 + x);        // x = 1 - x*x;
   lnx = logf(x);

   tt1 = 2.0f/((float)M_PI*0.147f) + 0.5f * lnx;
   tt2 = 1.0f/(0.147f) * lnx;

   return(sgn*sqrtf(-tt1 + sqrtf(tt1*tt1 - tt2)));
}

/* Assumues normal distribution with a mean of 0
	quantile(p) = x <=> The probability of distribution values falling below x is p 
*/
float Dist::quantile(float p, float varSqrt){
	return varSqrt*sqrt(2.0f)*Dist::invErf(2.0f*p - 1.0f);
}
