#pragma once

#define WIDTH 700
#define HEIGHT 700

#define DEBUG_VAR(x) cout << #x << ": " << x << endl
#define DEBUGL(x) cout << __FUNCTION__ << ": line " << __LINE__ << endl

struct rgb {
	float r, g, b;
};

struct mesh {
	GLfloat x, y, z, rotx, roty, rotz, shine;
	rgb amb, dif, spec;
};
