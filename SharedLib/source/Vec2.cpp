#include "Vec2.h"

template<class T>	
T Vec2<T>::manDist(){
	return x + y;
}

template<class T>
void Vec2<T>::add(Vec2<T> other){
	x += other.x;
	y += other.y;
}

template<class T>
void Vec2<T>::sub(Vec2<T> other){
	x -= other.x;
	y -= other.y;
}

template<class T>
bool Vec2<T>::equals(Vec2<T> other){
	return x == other.x && y == other.y;
}

template<class T>
void Vec2<T>::writeToStream(std::ofstream* outStream){
	outStream->write(reinterpret_cast<char*>(&x),	sizeof(T));
	outStream->write(reinterpret_cast<char*>(&y),	sizeof(T));
}

template<class T>
void Vec2<T>::readFromStream(std::ifstream* inStream){
	inStream->read(reinterpret_cast<char*>(&x),	sizeof(T));
	inStream->read(reinterpret_cast<char*>(&y),	sizeof(T));
}

template<class T>
Vec2<T> Vec2<T>::min(Vec2<T> a, Vec2<T> b){
	return Vec2<T>(	a.x < b.x ? a.x : b.x,
					a.y < b.y ? a.y : b.y);
}

template<class T>
Vec2<T> Vec2<T>::max(Vec2<T> a, Vec2<T> b){
	return Vec2<T>(	a.x > b.x ? a.x : b.x,
					a.y > b.y ? a.y : b.y);
}

template<class T>
Vec2<T> Vec2<T>::add(Vec2<T> a, Vec2<T> b){
	return Vec2<T>(a.x + b.x, a.y + b.y);
}

template<class T>
Vec2<T> Vec2<T>::sub(Vec2<T> a, Vec2<T> b){
	return Vec2<T>(a.x - b.x, a.y - b.y);
}

template<class T>
Vec2<T> Vec2<T>::compMult(Vec2<T> a, Vec2<T> b){
	return Vec2<T>(a.x*b.x, a.y*b.y);
}

template<class T>
bool Vec2<T>::equals(Vec2<T> a, Vec2<T> b){
	return a.x == b.x && a.y == b.y;
}

template class Vec2<int>;
template class Vec2<float>;