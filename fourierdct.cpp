#include "fourierdct.h"
#include "transformwindow.h"
#include "colorparser.h"
#include "photowindow.h"

#include <QDebug>

FourierDCT::FourierDCT(QObject *parent) :
	ImageTransformFilter(parent),
	mCA(NULL)
{
	test();
}

FourierDCT::~FourierDCT()
{
	delete mCA;
}

void FourierDCT::test()
{
	QVector<Complex> c;
	// http://www.hydrogenaudio.org/forums/index.php?showtopic=39574
	qDebug() << "FourierDCT test. expected values: 5.0000   -2.2304         0   -0.1585";
	c << Complex(1, 0);
	c << Complex(2, 0);
	c << Complex(3, 0);
	c << Complex(4, 0);
	qDebug() << "input: " << c;

	mAlphaDC = 1.0 / sqrt(c.count());
	mAlphaAC = sqrt(2.0 / c.count());
	prepareScale(c.size());
	qDebug() << mScale;

	ComplexArray *ca = new ComplexArray(boost::extents[1][1][c.count()]);
	for (int i = 0; i < c.count(); i += 1) {
		(*ca)[0][0][i] = Complex(c.at(i).real(), 0);
	}
	oneDFftV(ca, 0, 2, 1, false);

	rearrangeDct(c);
	rearrange(c);
	transform(c, false);
	qDebug() << "transformed by hand: " << c;


	for (int i = 0; i < c.count(); i++) {
		c[i] *= mScale.at(i);
	}
	qDebug() << "scaled by hand: " << c;

	for (int i = 0; i < c.count(); i++) {
		c[i] /= mScale.at(i);
	}
	qDebug() << "divided by hand: " << c;
	rearrange(c);
	transform(c, true);
	qDebug() << "inverted by hand: " << c;

	c.resize(0);
	for (unsigned int i = 0; i < ca->shape()[2]; i++) {
		c << (*ca)[0][0][i];
	}
	qDebug() << "transformed automatically: " << c;

	//perform(ca, true);
	oneDFftV(ca, 0, 2, 1, true);
	c.resize(0);
	for (unsigned int i = 0; i < ca->shape()[2]; i++) {
		c << (*ca)[0][0][i];
	}
	qDebug() << "inverted automatically: " << c;

	delete ca;
}

bool FourierDCT::setup(const FilterData &data)
{
	if (ImageTransformFilter::setup(data)) {
		if (mFormat == QImage::Format_ARGB32 || mFormat == QImage::Format_ARGB32_Premultiplied) {
			qWarning() << "transparancy in the image is discarded";
			mImg.convertToFormat(QImage::Format_RGB32);
		}
		mSize = QSize(1, 1);
		while (mSize.width() < mImg.width() || mSize.height() < mImg.height()) {
			mSize *= 2;
		}

		int layers;
		switch (mFormat) {
			case QImage::Format_Indexed8:
				layers = 1;
				break;
			case QImage::Format_ARGB32:
			case QImage::Format_RGB32:
				layers = 3;
				break;
			default:
				return false;
		}

		// the array created here might be bigger than image size - it has to be
		// a square of side length 2^n
		delete mCA;
		int w = mSize.width();
		int h = mSize.height();
		mCA = new ComplexArray(boost::extents[layers][w][h]);

		// fill only the data that exists in the image
		for (int i = 0; i < layers; i++) {
			for (int y = 0; y < mImg.height(); y++) {
				for (int x = 0; x < mImg.width(); x++) {
					(*mCA)[i][y][x] = Complex(extractColor(mImg.pixel(x, y), i), 0);
				}
			}
		}

		return true;
	} else {
		return false;
	}
}

QString FourierDCT::name() const
{
	return "FourierDCT";
}

DisplayWindow *FourierDCT::apply(QString windowBaseName)
{
	perform(mCA);
	int layers = mCA->shape()[0];
	int w = mSize.width();
	int h = mSize.height();
	ComplexArray *ca = new ComplexArray(boost::extents[layers][w][h]);
	*ca = *mCA;

	const QImages images = complexToImages(mCA, mFormat);

	// parent's parent should be MainWindow
	QWidget *mainWindow = q_check_ptr(qobject_cast<QWidget *>(parent()->parent()));

	// this object is used for transform inversion and can be shared among
	// multiple instances of TransformWindow* and hance cannot use one of them
	// as a parent. ideally, this should be constructed for each and every
	// instance of TW but since we do not store any actual data, the "leak"
	// (this is a QObject, so it adheres to QObject's deletion rules) is quite
	// small.
	// * - see TransformFilter class
	FourierDCT *dct = new FourierDCT(qobject_cast<QObject *>(mainWindow));
	return new TransformWindow(ca, dct, mFormat, windowBaseName + ", " + name(), mainWindow);
}

void FourierDCT::rearrange(QVector<Complex> &elements)
{
	int target = 0;
	for (int position = 0; position < elements.count(); position++) {
		if (target > position) {
			Complex temp = elements.at(position);
			elements[position] = elements.at(target);
			elements[target] = temp;
		}

		unsigned int mask = elements.count();

		while (target & (mask >>= 1)) {
			target &= ~mask;
		}

		target |= mask;
	}
}


void FourierDCT::rearrangeDct(QVector<Complex> &elements)
{
	QVector<Complex> result;
	for (int k = 0; k < elements.size(); k += 2) {
		result << elements.at(k);
	}
	for (int k = elements.size() - 1; k >= 0; k -= 2) {
		result << elements.at(k);
	}
	elements = result;
}

qreal FourierDCT::alpha(int u) const
{
	return (u == 0) ? mAlphaDC : mAlphaAC;
}

void FourierDCT::prepareScale(int n)
{
	for (int i = 0; i < n; i++) {
		mScale << Complex::fromPowerPhase(1, -M_PI_2 * i / n) * alpha(i);
	}
}

void FourierDCT::oneDFftH(ComplexArray *ca, int idx, int idx1, int idx2, bool inverse)
{
	for (unsigned int j = 0; j < ca->shape()[idx2]; j++) {
		QVector<Complex> elements;
		elements.reserve(ca->shape()[idx1]);
		for (unsigned int k = 0; k < ca->shape()[idx1]; k++) {
			elements << (*ca)[idx][k][j];
		}

		if (!inverse) {
			rearrangeDct(elements);
		}
		rearrange(elements);
		transform(elements, inverse);

		for (unsigned int k = 0; k < ca->shape()[idx1]; k++) {
			(*ca)[idx][k][j] = elements.at(k) * mScale.at(k);
		}
	}
}

void FourierDCT::oneDFftV(ComplexArray *ca, int idx, int idx1, int idx2, bool inverse)
{
	for (unsigned int j = 0; j < ca->shape()[idx2]; j++) {
		QVector<Complex> elements;
		elements.reserve(ca->shape()[idx1]);
		for (unsigned int k = 0; k < ca->shape()[idx1]; k++) {
			elements << (*ca)[idx][j][k];
		}

		if (!inverse) {
			rearrangeDct(elements);
		}
		rearrange(elements);
		transform(elements, inverse);

		for (unsigned int k = 0; k < ca->shape()[idx1]; k++) {
			(*ca)[idx][j][k] = elements.at(k) * mScale.at(k);
		}
	}
}

void FourierDCT::prepareFft(ComplexArray *ca, int idx, int idx1, int idx2)
{
	qreal c = -M_PI_2 / qreal(ca->shape()[idx1]);
	mAlphaDC = 1.0 / sqrt(ca->shape()[idx1]);
	mAlphaAC = sqrt(2.0 / ca->shape()[idx1]);
	mW.resize(0);
	for (unsigned int j = 0; j < ca->shape()[1]; j++) {
		mW << Complex::fromPowerPhase(1, c * j) * Complex(alpha(j), 0);
	}
	for (unsigned int j = 0; j < ca->shape()[idx2]; j++) {
		for (unsigned int k = 0; k < ca->shape()[idx1]; k++) {
			(*ca)[idx][k][j] *= mW.at(k);
		}
	}
}

void FourierDCT::perform(ComplexArray *ca, bool inverse)
{
	Q_ASSERT(ca->num_dimensions() == 3);
	mW.reserve(qMax(ca->shape()[1], ca->shape()[2]));
	for (unsigned int i = 0; i < ca->shape()[0]; i++) {

		if (inverse) {
			prepareFft(ca, i, 1, 2);
			prepareFft(ca, i, 2, 1);
		}

		oneDFftH(ca, i, 1, 2, inverse);
		oneDFftV(ca, i, 2, 1, inverse);

		if (!inverse) {
			prepareFft(ca, i, 1, 2);
			prepareFft(ca, i, 2, 1);
		}
	}
}

void FourierDCT::transform(QVector<Complex> &elements, bool inverse)
{
	const unsigned int N = elements.count();
	const qreal pi = inverse ? M_PI : -M_PI;
	for (unsigned int step = 1; step < N; step <<= 1) {
		const unsigned int jump = step << 1;
		const qreal delta = pi / qreal(step);
		const qreal sine = sin(delta * .5);
		const Complex multiplier(-2. * sine * sine, sin(delta));
		Complex factor(1, 0);

		for (unsigned int group = 0; group < step; ++group) {
			for (unsigned int pair = group; pair < N; pair += jump) {
				const unsigned int match = pair + step;
				const Complex product(factor * elements.at(match));
				elements[match] = elements.at(pair) - product;
				elements[pair] += product;
			}
			factor = multiplier * factor + factor;
		}
	}
	if (inverse) {
		qreal scale = 1.0 / elements.size();
		for (int i = 0; i < elements.size(); i++) {
			elements[i] *= scale;
		}
	}
}

DisplayWindow *FourierDCT::invert(ComplexArray *ca, QString title, QImage::Format format, QWidget *p)
{
	int w = ca->shape()[1];
	int h = ca->shape()[2];
	perform(ca, true);
	ColorParser cp(format);
	QImage result(w, h, format);
	result.fill(Qt::black);
	if (format == QImage::Format_Indexed8) {
		QVector<QRgb> colors;
		colors.reserve(256);
		for (int i = 0; i < 256; i++) {
			colors << qRgb(i, i, i);
		}
		result.setColorTable(colors);
	}
	for (unsigned int i = 0; i < ca->shape()[0]; i++) {
		qreal min = 0;
		qreal max = 0;
		for (unsigned int j = 0; j < ca->shape()[1]; j++) {
			for (unsigned int k = 0; k < ca->shape()[2]; k++) {
				qreal real = (*ca)[i][j][k].real();
				if (real > max) {
					max = real;
				} else if (real < min) {
					min = real;
				}
			}
		}

		for (unsigned int j = 0; j < ca->shape()[1]; j++) {
			for (unsigned int k = 0; k < ca->shape()[2]; k++) {
				qreal p = ((*ca)[i][j][k].real() - min) / (max - min) * 255.0;
				{
					QVector3D oldPixel = cp.pixel(k, j, result);
					QVector3D newPixel;
					switch (i) {
						case 0:
							newPixel.setX(p);
							break;
						case 1:
							newPixel.setY(p);
							break;
						case 2:
							newPixel.setZ(p);
							break;
						default:
							break;
					}
					cp.setPixel(k, j, result, cp.merge(oldPixel, newPixel));
				}
			}
		}
	}
	result = result.rgbSwapped();
	PhotoWindow *pw = new PhotoWindow(result, title + ", IFDCT", p);
	return pw;
}

const ImageTransformFilter::QImages FourierDCT::complexToImages(const ComplexArray *ca, QImage::Format format) const
{
	QSize size(ca->shape()[1], ca->shape()[2]);
	QImage phaseImage = QImage(size, format);
	QImage magnitudeImage = QImage(size, format);
	if (format == QImage::Format_Indexed8) {
		QVector<QRgb> colors;
		colors.reserve(256);
		for (int i = 0; i < 256; i++) {
			colors << qRgb(i, i, i);
		}
		phaseImage.setColorTable(colors);
		magnitudeImage.setColorTable(colors);
		phaseImage.fill(0);
		magnitudeImage.fill(0);
	} else {
		phaseImage.fill(Qt::black);
		magnitudeImage.fill(Qt::black);
	}
	for (unsigned int i = 0; i < ca->shape()[0]; i++) {
		qreal minp = 0;
		qreal maxp = 0;
		qreal minm = 0;
		qreal maxm = 0;
		for (unsigned int j = 0; j < ca->shape()[1]; j++) {
			for (unsigned int k = 0; k < ca->shape()[2]; k++) {
				qreal phase = (*ca)[i][j][k].phase();
				qreal magnitude = (*ca)[i][j][k].abs();
				if (phase > maxp) {
					maxp = phase;
				} else if (phase < minp) {
					minp = phase;
				}
				if (magnitude > maxm) {
					maxm = magnitude;
				} else if (magnitude < minm) {
					minm = magnitude;
				}
			}
		}

		ColorParser cp(format);
		for (unsigned int j = 0; j < ca->shape()[1]; j++) {
			for (unsigned int k = 0; k < ca->shape()[2]; k++) {
				qreal p = ((*ca)[i][j][k].phase() - minp) / (maxp - minp) * 255.0;
				{
					QVector3D oldPixel = cp.pixel(k, j, phaseImage);
					QVector3D newPixel;
					switch (i) {
						case 0:
							newPixel.setX(p);
							break;
						case 1:
							newPixel.setY(p);
							break;
						case 2:
							newPixel.setZ(p);
							break;
						default:
							break;
					}
					cp.setPixel(k, j, phaseImage, cp.merge(oldPixel, newPixel));
				}

				// logarithmic scale
				// implementaion: http://homepages.inf.ed.ac.uk/rbf/HIPR2/pixlog.htm
				// idea: http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm#guidelines
				p = (*ca)[i][j][k].abs();
				{
					qreal c = 255.0 / log(1.0 + abs(maxm - minm));
					p = c * log(1.0 + p);
					QVector3D oldPixel = cp.pixel(k, j, magnitudeImage);
					QVector3D newPixel;
					switch (i) {
						case 0:
							newPixel.setX(p);
							break;
						case 1:
							newPixel.setY(p);
							break;
						case 2:
							newPixel.setZ(p);
							break;
						default:
							break;
					}
					cp.setPixel(k, j, magnitudeImage, cp.merge(oldPixel, newPixel));
				}
			}
		}
	}
	//rearrangeQuadrants(phaseImage, magnitudeImage);
	QImages images;
	images.magnitude = magnitudeImage;
	images.phase = phaseImage;
	return images;
}
