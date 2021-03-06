#include "bandpassfilter.h"
#include "multislidingvaluedialog.h"
#include "transformwindow.h"

#include <QMessageBox>

BandPassFilter::BandPassFilter(QObject *parent) :
	TransformFilter(parent)
{
}

QString BandPassFilter::name() const
{
	return "Band pass";
}

bool BandPassFilter::setup(const FilterData &data)
{
	int radiusMax = data.transformData.shape()[1] / 2 * M_SQRT2l;
	MultiSlidingValueDialog svd(name() + " %2, x: %1", 0, 0, radiusMax, 2,
						   qobject_cast<QWidget *>(this));
	if (svd.exec() != QDialog::Accepted) {
		return false;
	}
	mRadiusNear = svd.values().at(0) * svd.values().at(0);
	mRadiusFar = svd.values().at(1) * svd.values().at(1);
	if (mRadiusNear > mRadiusFar) {
		QMessageBox::warning(qobject_cast<QWidget *>(this), "Condition not met",
							 "Condition radius1 <= radius2 not met. Aborting");
		return false;
	}
	mFormat = data.image.format();
	return TransformFilter::setup(data);
}

DisplayWindow *BandPassFilter::apply(QString windowBaseName)
{
	Q_ASSERT(mCA->dimensionality == 3);
	QVector2D center(mCA->shape()[1] / 2, mCA->shape()[2] / 2);
	rearrangeQuadrants();
	for (unsigned int i = 0; i < mCA->shape()[0]; i++) {
		for (unsigned int j = 0; j < mCA->shape()[1]; j++) {
			for (unsigned int k = 0; k < mCA->shape()[2]; k++) {
				QVector2D point(j, k);
				int freq = (point - center).lengthSquared();
				if (freq < mRadiusNear || freq > mRadiusFar) {
					(*mCA)[i][j][k] = Complex();
				}
			}
		}
	}
	rearrangeQuadrants();
	int layers = mCA->shape()[0];
	int w = mCA->shape()[1];
	int h = mCA->shape()[2];
	ComplexArray *ca = new ComplexArray(boost::extents[layers][w][h]);
	*ca = *mCA;
	TransformWindow *tw = q_check_ptr(qobject_cast<TransformWindow *>(parent()));
	ImageTransformFilter *inv = tw->inverter();
	return new TransformWindow(ca, inv, mFormat, windowBaseName + ", " + name(),
							   q_check_ptr(qobject_cast<QWidget *>(tw->parent())));
}
