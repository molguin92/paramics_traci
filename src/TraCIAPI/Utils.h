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

class PositionalData: Vector3D
{
public:
	double gradient;
	double bearing;

	PositionalData(double x, double y, double z, double b, double g): Vector3D(x, y, z), gradient(g), bearing(b)
	{
	}
};

class DimensionalData
{
public:
	double height;
	double length;
	double width;

	DimensionalData(double h, double l, double w): height(h), length(l), width(w)
	{
	}
};
