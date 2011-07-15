#pragma once

#include "imageloader.h"


#define DEBUG_VAR(x) cout << #x << ": " << x << endl
#define DEBUGL(x) cout << __FUNCTION__ << ": line " << __LINE__ << endl

#define absf(x) ((x)<0? -(x) : (x))

struct rgb_t {
	float r, g, b;
};

#define MPOS(m,x,y) m[x][y]

#define WIDTH 700
#define HEIGHT 700


class DofFilter
{
	public:
		DofFilter(float near, float far);
		~DofFilter();
		
		bool	useKernel, boostBokeh;
		
		float 	db[WIDTH][HEIGHT];
		rgb_t 	fb1[WIDTH][HEIGHT], fb2[WIDTH][HEIGHT];
		int		sb[WIDTH][HEIGHT];
		
		void 	apply();
		void 	setParams(float focus, float focalLen, float aperture);
		
		void	updateDistance();
		void 	allocMatrixes(int w, int h);
		void	loadKernelBMP(const char* filename);
		
	
	private:
		float focus, focalLen, aperture, sensorDist;
		float near, far;
		Image *kernel;
		
		inline void 	filterPixelOnBuffer( float r, int x, int y );
		inline double 	kernelVal(int x, int y);
		inline double 	biInterpolKernel( int s, int t, double kratio );
		void 			printKernel();

};
