#include "complex.h"

#include <cmath>
#include <QDebug>

/*
 * see http://www.cs.caltech.edu/courses/cs11/material/cpp/donnie/cpp-ops.html
 */

Complex::Complex()
{
}

Complex::Complex(const Complex &other)
{
	this->mNumber = other.mNumber;
}

Complex::Complex(const qreal &real, const qreal &imaginary) :
	mNumber(QVector2D(real, imaginary))
{
}

Complex Complex::fromPowerPhase(const qreal &power, const qreal &phase)
{
	return Complex(power * cos(phase), power * sin(phase));
}

qreal Complex::abs() const
{
	return mNumber.length();
}

qreal Complex::real() const
{
	return mNumber.x();
}

qreal Complex::imaginary() const
{
	return mNumber.y();
}

void Complex::setReal(const qreal &real)
{
	mNumber.setX(real);
}

void Complex::setImaginary(const qreal &imaginary)
{
	mNumber.setY(imaginary);
}

Complex &Complex::operator =(const Complex &rhs)
{
	// it is safe to skip self-assignment check here
	this->mNumber = rhs.mNumber;
	return *this;
}

Complex &Complex::operator+=(const Complex &rhs)
{
	this->mNumber += rhs.mNumber;
	return *this;
}

Complex &Complex::operator-=(const Complex &rhs)
{
	this->mNumber -= rhs.mNumber;
	return *this;
}

Complex &Complex::operator*=(const Complex &rhs)
{
	this->mNumber *= rhs.mNumber;
	return *this;
}

const Complex Complex::operator +(const Complex &rhs)
{
	return Complex(*this) += rhs;
}

const Complex Complex::operator -(const Complex &rhs)
{
	return Complex(*this) -= rhs;
}

const Complex Complex::operator *(const Complex &rhs)
{
	return Complex(*this) *= rhs;
}

bool Complex::operator ==(const Complex &rhs)
{
	return this->mNumber == rhs.mNumber;
}

bool Complex::operator !=(const Complex &rhs)
{
	return this->mNumber != rhs.mNumber;
}

QDebug operator <<(QDebug &stream, const Complex &rhs)
{
	stream << "Complex(" << rhs.mNumber.x() << "," << rhs.mNumber.y() << ")";
	return stream.space();
}