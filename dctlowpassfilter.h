#ifndef DCTLOWPASSFILTER_H
#define DCTLOWPASSFILTER_H

#include "transformfilter.h"

class DctLowPassFilter : public TransformFilter
{
	Q_OBJECT
public:
	explicit DctLowPassFilter(QObject *parent = 0);
	virtual QString name() const;

signals:

public slots:
	virtual bool setup(const FilterData &data);
	virtual DisplayWindow *apply(QString windowBaseName);

private:
	int mRadius;
	QImage::Format mFormat;
};

#endif // DCTLOWPASSFILTER_H
