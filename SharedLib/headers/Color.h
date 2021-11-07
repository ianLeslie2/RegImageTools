#ifndef COLOR_H
#define COLOR_H

#define COLOR_EPSILON 0.001

class Color
{
	public:
		double x;
		double y;
		double z;
		
		Color();
		Color(const Color& other);
		Color(double x, double y, double z);
		
		bool equals(Color other);
		char* toStr();
		
		double _gammaExpansion(double n);
		double _gammaCompression(double n);
		
		void multMatrix(double a1, double a2, double a3,
						double b1, double b2, double b3,
						double c1, double c2, double c3);
		void convert_sRGB_XYZ();
		void convert_XYZ_sRGB();
		void convert_XYZ_LAB();
		void convert_LAB_XYZ();
		
		static float getManhattanDiff(Color c1, Color c2);
		static float getDiff(Color c1, Color c2);
		// The weights are relative and dont need to add up to 1
		static Color getComb(Color c1, float w1, Color c2, float w2);
};


#endif