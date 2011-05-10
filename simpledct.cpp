#include "simpledct.h"
#include "transformwindow.h"
#include "colorparser.h"

#include <QDebug>

SimpleDCT::SimpleDCT(QObject *parent) :
	ImageTransformFilter(parent),
	mCA(NULL)
{
	test();
}

SimpleDCT::~SimpleDCT()
{
	delete mCA;
}

void SimpleDCT::test()
{
	QVector<Complex> c;
	// http://www.hydrogenaudio.org/forums/index.php?showtopic=39574
	qDebug() << "SimpleDCT test. expected values: 5.0000   -2.2304         0   -0.1585";
	c << Complex(1, 0);
	c << Complex(2, 0);
	c << Complex(3, 0);
	c << Complex(4, 0);
	qDebug() << "input: " << c;

	ComplexArray *ca = new ComplexArray(boost::extents[1][1][c.count()]);
	for (int i = 0; i < c.count(); i++) {
		(*ca)[0][0][i] = c.at(i);
	}
	perform(ca);

	transform(c, false);
	qDebug() << "transformed by hand: " << c;

	bool matlab = false;
	qreal fac;

	if (!matlab) {
		mAlphaDC = 1.0 / sqrt(2);
		mAlphaAC = 1;
		fac = sqrt(2.0 / c.count());
	} else {
		// matlab version:
		mAlphaDC = 1.0 / sqrt(c.count());
		mAlphaAC = sqrt(2.0 / c.count());
		fac = 1;
	}

	for (int i = 0; i < c.count(); i++) {
		c[i] *= Complex(alpha(i) * fac, 0);
	}
	qDebug() << "scaled by hand: " << c;

	for (int i = 0; i < c.count(); i++) {
		c[i] *= Complex(alpha(i), 0);
	}
	transform(c, true);
	if (!matlab) {
		for (int i = 0; i < c.count(); i++) {
			c[i] *= Complex(sqrt(2.0 / c.count()), 0);
		}
	}
	qDebug() << "inverted by hand: " << c;

	c.resize(0);
	for (unsigned int i = 0; i < ca->shape()[3]; i++) {
		c << (*ca)[0][0][i];
	}
	qDebug() << "transformed automatically: " << c;

	perform(ca, true);
	c.resize(0);
	for (unsigned int i = 0; i < ca->shape()[3]; i++) {
		c << (*ca)[0][0][i];
	}
	qDebug() << "inverted automatically: " << c;

	delete ca;
}

bool SimpleDCT::setup(const FilterData &data)
{
	if (ImageTransformFilter::setup(data)) {
		if (mFormat == QImage::Format_ARGB32 || mFormat == QImage::Format_ARGB32_Premultiplied) {
			qWarning() << "transparancy in the image is discarded";
			mImg.convertToFormat(QImage::Format_RGB32);
		}
		mSize = mImg.size();

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

		delete mCA;

		int w = mSize.width();
		int h = mSize.height();
		mCA = new ComplexArray(boost::extents[layers][w][h]);
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

QString SimpleDCT::name() const
{
	return "Simple DCT";
}

DisplayWindow *SimpleDCT::apply(QString windowBaseName)
{
	perform(mCA);
	int layers = mCA->shape()[0];
	int w = mSize.width();
	int h = mSize.height();
	ComplexArray *ca = new ComplexArray(boost::extents[layers][w][h]);
	*ca = *mCA;

	// parent's parent should be MainWindow
	QWidget *mainWindow = q_check_ptr(qobject_cast<QWidget *>(parent()->parent()));

	// see the comment in FFT::apply
	SimpleDCT *sdct = new SimpleDCT(qobject_cast<QObject *>(mainWindow));
	return new TransformWindow(ca, sdct, mFormat, windowBaseName + ", " + name(), q_check_ptr(qobject_cast<QWidget *>(parent()->parent())));
}

void SimpleDCT::perform(ComplexArray *ca, bool inverse)
{
	Q_ASSERT(ca->num_dimensions() == 3);
	for (unsigned int i = 0; i < ca->shape()[0]; i++) {
		if (inverse) {
			// alpha values get the N = (size of the innermost loop)
			mAlphaDC = 1.0 / sqrt(ca->shape()[1]);
			mAlphaAC = sqrt(2.0 / ca->shape()[1]);
			for (unsigned int j = 0; j < ca->shape()[2]; j++) {
				for (unsigned int k = 0; k < ca->shape()[1]; k++) {
					(*ca)[i][k][j] *= Complex(alpha(k), 0);
				}
			}

			mAlphaDC = 1.0 / sqrt(ca->shape()[2]);
			mAlphaAC = sqrt(2.0 / ca->shape()[2]);
			for (unsigned int j = 0; j < ca->shape()[1]; j++) {
				for (unsigned int k = 0; k < ca->shape()[2]; k++) {
					(*ca)[i][j][k] *= Complex(alpha(k), 0);
				}
			}
		}

		for (unsigned int j = 0; j < ca->shape()[2]; j++) {
			QVector<Complex> elements;
			elements.reserve(ca->shape()[1]);
			for (unsigned int k = 0; k < ca->shape()[1]; k++) {
				elements << (*ca)[i][k][j];
			}
			transform(elements, inverse);
			for (unsigned int k = 0; k < ca->shape()[1]; k++) {
				(*ca)[i][k][j] = elements.at(k);
			}
		}

		for (unsigned int j = 0; j < ca->shape()[1]; j++) {
			QVector<Complex> elements;
			elements.reserve(ca->shape()[2]);
			for (unsigned int k = 0; k < ca->shape()[2]; k++) {
				elements << (*ca)[i][j][k];
			}
			transform(elements, inverse);
			for (unsigned int k = 0; k < ca->shape()[2]; k++) {
				(*ca)[i][j][k] = elements.at(k);
			}
		}

		if (!inverse) {
			// alpha values get the N = (size of the innermost loop)
			mAlphaDC = 1.0 / sqrt(ca->shape()[1]);
			mAlphaAC = sqrt(2.0 / ca->shape()[1]);
			for (unsigned int j = 0; j < ca->shape()[2]; j++) {
				for (unsigned int k = 0; k < ca->shape()[1]; k++) {
					(*ca)[i][k][j] *= Complex(alpha(k), 0);
				}
			}

			mAlphaDC = 1.0 / sqrt(ca->shape()[2]);
			mAlphaAC = sqrt(2.0 / ca->shape()[2]);
			for (unsigned int j = 0; j < ca->shape()[1]; j++) {
				for (unsigned int k = 0; k < ca->shape()[2]; k++) {
					(*ca)[i][j][k] *= Complex(alpha(k), 0);
				}
			}
		}
	}
}

qreal SimpleDCT::alpha(int u) const
{
	return (u == 0) ? mAlphaDC : mAlphaAC;
}

void SimpleDCT::transform(QVector<Complex> &elements, bool inverse)
{
	Q_UNUSED(inverse);
	QVector<Complex> result;
	const int N = elements.count();
	result.reserve(N);
	for (int i = 0; i < N; i++) {
		Complex c;
		for (int j = 0; j < N; j++) {
			qreal val = elements.at(j).real();
			qreal fac1, fac2;
			if (!inverse) {
				fac1 = (2 * j + 1);
				fac2 = i;
			} else {
				fac1 = (2 * i + 1);
				fac2 = j;
			}
			c.setReal(val * std::cos(M_PI_2 * fac1 * fac2 / N) + c.real());
		}
		result << c;
	}
	elements = result;
}

const ImageTransformFilter::QImages SimpleDCT::complexToImages(const ComplexArray *ca, QImage::Format format) const
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
	QImages images;
	images.magnitude = magnitudeImage;
	images.phase = phaseImage;
	return images;
}
