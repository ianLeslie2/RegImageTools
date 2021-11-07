#pragma once
#include <fstream>

template<class T>
class Vec2{
	public:
		T x;
		T y;
		
		Vec2<T>(){
			this->x = 0;
			this->y = 0;
		}
		Vec2<T>(T x, T y){
			this->x = x;
			this->y = y;
		}
		
		T manDist();
		void add(Vec2<T> other);
		void sub(Vec2<T> other);
		bool equals(Vec2<T> other);
		
		void writeToStream(std::ofstream* outStream);
		void readFromStream(std::ifstream* inStream);
		
		static Vec2<T> min(Vec2<T> a, Vec2<T> b);
		static Vec2<T> max(Vec2<T> a, Vec2<T> b);
		static Vec2<T> add(Vec2<T> a, Vec2<T> b);
		static Vec2<T> sub(Vec2<T> a, Vec2<T> b);
		static Vec2<T> compMult(Vec2<T> a, Vec2<T> b);
		static bool equals(Vec2<T> a, Vec2<T> b);
};