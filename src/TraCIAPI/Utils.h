#pragma once

class Vector2D
{
public:
	double x;
	double y;

	Vector2D(double x, double y) : x(x), y(y)
	{
	}
};

class Vector3D: Vector2D
{
public:
	double z;

	Vector3D(double x, double y, double z): Vector2D(x, y), z(z)
	{
	}
};
