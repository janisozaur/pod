#include "simpledct.h"
#include "transformwindow.h"

#include <QDebug>

SimpleDCT::SimpleDCT(QObject *parent) :
	ImageTransformFilter(parent),
	mCA(NULL)
{
}

SimpleDCT::~SimpleDCT()
{
	delete mCA;
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
	return new TransformWindow(ca, mFormat, windowBaseName + ", " + name(), q_check_ptr(qobject_cast<QWidget *>(parent()->parent())));
}

void SimpleDCT::perform(ComplexArray *ca, bool inverse)
{
	Q_ASSERT(ca->num_dimensions() == 3);
	for (unsigned int i = 0; i < ca->shape()[0]; i++) {
		// alpha values get the N = (size of the innermost loop)
		mAlphaDC = 1.0 / sqrt(ca->shape()[1]);
		mAlphaAC = sqrt(2.0 / ca->shape()[1]);
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

		mAlphaDC = 1.0 / sqrt(ca->shape()[2]);
		mAlphaAC = sqrt(2.0 / ca->shape()[2]);
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
			c.setReal(elements.at(j).real() * std::cos(M_PI_2 * (2 * j + 1) * i / N) + c.real());
		}
		c.setReal(c.real() * alpha(i));
		result << c;
	}
	elements = result;
}
