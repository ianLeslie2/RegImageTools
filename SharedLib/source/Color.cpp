#include "Color.h"
#include <cstdlib>
#include <math.h>
#include <cmath>
#include "UtilF.h"
#include <iostream>

/*
	NOTE:
		This color class is not reliant on a specific color space*
		It just expects a 3D color space
		
		The difference calculation is based on euclidean distance
		within the 3D space, and the blend operation is performed
		by linearly interpolating between the two 3D points.
		
		The blend & difference calculations are most perceptucally
		accurate for a color space like CIELAB, but it works for RGB as well
		
		(probably won't work for HSV tho since I don't think you can't
			just average the hue component)
		
		* However since the currently used color-space does not get tracked
		  conversion functions like convert_sRGB_XYZ do assume the sRGB space is being used
		  (and values are in the range 0-255)
*/

Color::Color(){
	x = 0;
	y = 0;
	z = 0;
}

Color::Color(const Color& other){
	x = other.x;
	y = other.y;
	z = other.z;
}

Color::Color(double x, double y, double z){
	this->x = x;
	this->y = y;
	this->z = z;
}

#define fuzzyEq(a,b) (((a)-(b)) <= COLOR_EPSILON && ((a)-(b)) >= -COLOR_EPSILON)

bool Color::equals(Color other){
	return fuzzyEq(x, other.x) && fuzzyEq(y, other.y) && fuzzyEq(z, other.z);
}

char* Color::toStr() {
	char buffer[STR_BUFFER_SIZE];
	sprintf(buffer, "(%.2f,%.2f,%.2f)", x, y, z);
	return UtilF::copyStr(buffer);
}

/*
 * gamma formulas from:
 * https://en.wikipedia.org/wiki/SRGB#The_forward_transformation_(CIE_XYZ_to_sRGB)
 */
// Input: 0-255, Output: 0-1
double Color::_gammaExpansion(double n){
	n = n/255.0;
	if(n <= 0.04045){
		return n/12.92;
	}
	else{
		return pow((n + 0.055) / 1.055, 2.4);
	}
}
// Input: 0-1, Output: 0-255
double Color::_gammaCompression(double n){
	if(n <= 0.0031308){
		n = n*12.92;
	}
	else{
		n = (1.055*pow(n, 1.0/2.4)) - 0.055;
	}
	n = n*255.0;
	if(n < 0){
		return 0;
	}
	else if(n > 255){
		return 255;
	}
	else{
		return round(n);
	}
}

/*
 * sRGB-XYZ conversion
 * https://mina86.com/2019/srgb-xyz-conversion/
 */
void Color::multMatrix(double a1, double a2, double a3,
				double b1, double b2, double b3,
				double c1, double c2, double c3){
	double x_t = (a1*x + a2*y + a3*z);
	double y_t = (b1*x + b2*y + b3*z);
	double z_t = (c1*x + c2*y + c3*z);
	x = x_t;
	y = y_t;
	z = z_t;
}
// Using D65 reference white
// These matrixes are also listed here http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
void Color::convert_sRGB_XYZ(){
	x = _gammaExpansion(x);
	y = _gammaExpansion(y);
	z = _gammaExpansion(z);
	multMatrix(0.4123865632529917,   0.35759149092062537, 0.18045049120356368,
				0.21263682167732384,  0.7151829818412507,  0.07218019648142547,
				0.019330620152483987, 0.11919716364020845, 0.9503725870054354);
}
void Color::convert_XYZ_sRGB(){
	multMatrix(3.2410032329763587,   -1.5373989694887855,  -0.4986158819963629,
				-0.9692242522025166,    1.875929983695176,    0.041554226340084724,
				 0.055639419851975444, -0.20401120612390997,  1.0571489771875335);
	x = _gammaCompression(x);
	y = _gammaCompression(y);
	z = _gammaCompression(z);
}

/*
 * XYZ-LAB conversion
 * http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Lab.html
 * http://www.easyrgb.com/en/math.php
 *
 * The formula listed on this one is wrong -> http://www.brucelindbloom.com/index.html?Eqn_Lab_to_XYZ.html
 * Feel like I should give negative credit for that.
 */
void Color::convert_XYZ_LAB(){
	// Reference white values for 2 deg standard 
	// https://en.wikipedia.org/wiki/Illuminant_D65
	double refX = 95.047;
	double refY = 100.0;
	double refZ = 108.883;
	
	double eps = 216.0/24389.0;
	double k = 24389.0/27.0;
	
	double x_r = x/refX;
	double y_r = y/refY;
	double z_r = z/refZ;
	
	double fx, fy, fz;
	if(x_r > eps) fx = pow(x_r,1.0/3.0);
	else fx = (k*x_r + 16.0)/116.0;
	if(y_r > eps) fy = pow(y_r,1.0/3.0);
	else fy = (k*y_r + 16.0)/116.0;
	if(z_r > eps) fz = pow(z_r,1.0/3.0);
	else fz = (k*z_r + 16.0)/116.0;
	
	// x,y,z = L,a,b
	x = 116.0*fy - 16.0;
	y = 500.0*(fx - fy);
	z = 200.0*(fy - fz);
}

void Color::convert_LAB_XYZ(){
	double L = x;
	double A = y;
	double B = z;
	
	// D65
	double refX = 95.047;
	double refY = 100.0;
	double refZ = 108.883;
	
	double eps = 216.0/24389.0;
	double k = 24389.0/27.0;
	
	double fy = (L + 16.0)/116.0;
	double fz = fy - (B/200.0);
	double fx = A/500.0 + fy;
	
	double x_r,y_r,z_r;
	if(pow(fx,3.0) > eps) x_r = pow(fx,3.0);
	else x_r = (116.0*fx - 16.0)/k;
	if(pow(fy,3.0) > eps) y_r = pow(fy,3.0);
	else y_r = (116.0*fy - 16.0)/k;
	if(pow(fz,3.0) > eps) z_r = pow(fz,3.0);
	else z_r = (116.0*fz - 16.0)/k;
	
	x = x_r*refX;
	y = y_r*refY;
	z = z_r*refZ;
}

//-------------------------------------------------------------------

float Color::getManhattanDiff(Color c1, Color c2) {
	return (float)UtilF::abs(c1.x - c2.x) + (float)UtilF::abs(c1.y - c2.y) + (float)UtilF::abs(c1.z - c2.z);
}


float Color::getDiff(Color c1, Color c2){
	/*
		Ideally we want to calculate the euclidean distance
		however performing a square root operation here
		slows down the whole process a lot, so that's not an option.
		
		We also can't exclude the square-root operation because
		a later calculation looks like: diff1*size1 + diff2*size2
		If diff is the perceptual difference squared, then color difference
		starts to overweight the amount of area affected making it less likely
		that a large region will absorb a small differently colored one.
		This results in a lot more high-contrast features in areas that degrade
		the quality of the image.
		
		So we want an approximation of euclidean distance, and the error must
		scale linearlly.
		
		For the analysis below I will consider the space 0,0,0 -> 1,1,1
		
		Option 1: Manhatten Distance (x+y+z)
			Low error when two components are small
			Highest error at (1,1,1), where error is 3-sqrt(3) ~= 1.27
			
		Option 2: Maximum component Max(x,y,z)
			Low error when two components are small
			Highest error at (1,1,1), where error is 1 - sqrt(3) ~= -0.73
			However this method has the drawback that changes to the non-maximum
			component don't change the diff score at all.
		
		Option 3: Combination of the above [(x+y+z) + Max(x,y,z)]/2
			Low error when two components are small
			Highest expected error at (1,1,1), where error is (3+1)/2 - sqrt(3) ~= 0.27
			I've tested this formula experimentally using a python script,
			the error range was 0->0.28 and the average error was 0.16
			so this seems to be a solid formula with no nasty edge cases
			
			And since theres the (x+y+z) part, any incremental color change changes the result.
			
		Option 4: Above but with adjusted weights 0.39*(x+y+z) + 0.61*Max(x,y,z)
			(weights were experimentally determined)
			Experimental error range (-0.034,0.154), spread of 0.188
	*/
	// TODO: Although this method is cute, I'd probably get better results using a known approximation method for the square root.
	float xDiff = (float)std::abs(c1.x - c2.x);
	float yDiff = (float)std::abs(c1.y - c2.y);
	float zDiff = (float)std::abs(c1.z - c2.z);
	
	float maxDiff = xDiff;
	if(yDiff > xDiff){
		if(zDiff > yDiff)
			maxDiff = zDiff;
		else
			maxDiff = yDiff;
	}
	else if(zDiff > xDiff){
		maxDiff = zDiff;
	}
	
	return 0.39f*(xDiff + yDiff + zDiff) + 0.61f*maxDiff;
}

Color Color::getComb(Color c1, float w1, Color c2, float w2){
	// Normalize weights
	float wSum = (w1 + w2);
	w1 /= wSum;
	w2 /= wSum;
	
	// Perform weighted linear interpolation
	return Color(
				w1*c1.x + w2*c2.x,
				w1*c1.y + w2*c2.y,
				w1*c1.z + w2*c2.z
				);
}