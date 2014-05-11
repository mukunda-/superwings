#ifndef XMATH_H
#define XMATH_H

static inline int getdistance( int x1,int y1, int x2, int y2 )
{
	return (x2-x1) * (x2-x1) + (y2-y1)* (y2-y1);
}

static inline int absi( int v )
{
	return v < 0 ? -v : v;
}

int decimal_adjust( int value );

#endif
