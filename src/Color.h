#ifndef __COLOR_H_
#define __COLOR_H_

#include "Vector.h"

template <class MapType> class Map;
typedef unsigned char Byte;

typedef Map<Byte>   BitMap;
typedef Map<double> DoubleMap;
typedef Map<float>  FMap;

template <class MapType>
class Map{
	MapType* _map;
	int _w, _h, _c;
public:
	Map(int w, int h, int c, Byte* m = 0){
		_w = w; _h = h; _c = c;
		_map = (m)? m : (new Byte[_w*_h*_c]); 
	}

	MapType* get(){return _map;}

	MapType* get(int x, int y){
		x = ((x<0)?0:( ( x>(_w-1) )? (_w-1) :x));
		y = (_h-1) - ((y<0)?0:( ( y>(_h-1))? (_h-1):y));
		return &_map[(y*_w + x)*_c];
	}

	int W(){return _w;}
	int H(){return _h;}
	int C(){return _c;}


};



static Byte* toByte(const RGB& c){
	Byte* col = new Byte[3];
	col[0] = (Byte)( ((c.x<0)?0:((c.x>1)?1:c.x)) * 255);
	col[1] = (Byte)( ((c.y<0)?0:((c.y>1)?1:c.y)) * 255);
	col[2] = (Byte)( ((c.z<0)?0:((c.z>1)?1:c.z)) * 255);
	return col;
};

static Byte* toByte(const RGBA& c){
	Byte* col = new Byte[3];
	col[0] = (Byte)( ((c.x<0)?0:((c.x>1)?1:c.x)) * 255);
	col[1] = (Byte)( ((c.y<0)?0:((c.y>1)?1:c.y)) * 255);
	col[2] = (Byte)( ((c.z<0)?0:((c.z>1)?1:c.z)) * 255);
	col[3] = (Byte)( ((c.w<0)?0:((c.w>1)?1:c.w)) * 255);
	return col;
};

static RGB toRGB(const Byte* c){
	return RGB(c[0]/255.0, c[1]/255.0, c[2]/255.0);
}

static RGBA toRGBA(const Byte* c){
	return RGBA(c[0]/255.0, c[1]/255.0, c[2]/255.0, c[3]/255.0);
}

static HSV toHSV(float r, float g, float b){

	float h,s,v;
	float min, max, delta;
	min = (r<g)?r:g; 
	min = (min<b)?min:b;
	max = (r>g)?r:g; 
	max = (max>b)?max:b; 
	v = max;				// v
	delta = max - min;
	if( max != 0 )
		s = delta / max;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		s = 0;
		h = -1;
		return HSV(h,s,v);
	}
	if( r == max )
		h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max )
		h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		h = 4 + ( r - g ) / delta;	// between magenta & cyan
	h *= 60;				// degrees
	if( h < 0 )
		h += 360;

	return HSV(h,s,v);
};

static HSV toHSV(const RGB& c){
	return toHSV(c.x, c.y, c.z);
};

static HSV toHSV(const RGBA& c){
	return toHSV(c.x, c.y, c.z);
};

static RGB toRGB(float h, float s, float v )
{
	int i;
	float r, g, b;
	float f, p, q, t;
	if( s == 0 ) {
		// achromatic (grey)
		r = g = b = v;
		return RGB(r,g,b);
	}
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		default:		// case 5:
			r = v;
			g = p;
			b = q;
			break;
	}

	return RGB(r,g,b);
};

static RGB toRGB(const HSV& col){
	return toRGB(col.x, col.y, col.z);
};

/*
struct RGBA{
	float R,G,B,A;

	RGBA(){
		R = 0; G = 0; B = 0; A = 1.0;
	}

	RGBA(float r, float g, float b, float a){
		R = r; G = g; B = b; A = a;
	}

	RGBA(float r, float g, float b){
		R = r; G = g; B = b; A = 1.0;
	}

	RGBA mixed(const RGBA& c, float t=0.5){
		return RGBA( R*(1-t)+c.R*t, G*(1-t)+c.G*t, B*(1-t)+c.B*t, A*(1-t)+c.A*t );
	}

	RGBA mix(const RGBA& c, float t=0.5){
		R = R*(1-t)+c.R*t; G= G*(1-t)+c.G*t; B = B*(1-t)+c.B*t; A = A*(1-t)+c.A*t;
	}

  friend RGBA operator-(const RGBA& v1);  // unary negation
  RGBA operator+(const RGBA& v2) const; // addition 
  friend RGBA operator-(const RGBA& v1, const RGBA& v2); //subtract
  friend RGBA operator*(const RGBA& v, double s);	// multiply
  friend RGBA operator*(double s, const RGBA& v);
  friend double   operator*(const RGBA& v1, const RGBA& v2); // dot
  friend RGBA operator^(const RGBA& v1, const RGBA& v2); // compt *
  friend RGBA operator%(const RGBA& v1, const RGBA& v2); // cross
  friend RGBA operator/(const RGBA& v, double s); // divide by scalar
  friend short    operator==(const RGBA& one, const RGBA& two); // equ

};*/

#endif