#ifndef DCTBANDSTOPFILTER_H
#define DCTBANDSTOPFILTER_H

#include "transformfilter.h"

class DctBandStopFilter : public TransformFilter
{
	Q_OBJECT
public:
	explicit DctBandStopFilter(QObject *parent = 0);
	virtual QString name() const;

signals:

public slots:
	virtual bool setup(const FilterData &data);
	virtual DisplayWindow *apply(QString windowBaseName);

private:
	int mRadiusNear, mRadiusFar;
	QImage::Format mFormat;
};

#endif // DCTBANDSTOPFILTER_H
