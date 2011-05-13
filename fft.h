#ifndef FFT_H
#define FFT_H

#include "imagetransformfilter.h"
#include "complexarray.h"

class FFT : public ImageTransformFilter
{
	Q_OBJECT
public:
	explicit FFT(QObject *parent = 0);
	~FFT();
	virtual QString name() const;
	void perform(ComplexArray *ca, bool inverse = false);
	virtual DisplayWindow *invert(ComplexArray *ca, QString title, QImage::Format format, QWidget *parent);
	virtual const QImages complexToImages(const ComplexArray *ca, QImage::Format format) const;

signals:

public slots:
	virtual DisplayWindow *apply(QString windowBaseName);
	virtual bool setup(const FilterData &data);

private:
	void rearrange(QVector<Complex> &elements);
	void transform(QVector<Complex> &elements, bool inverse);
	void oneDFftH(ComplexArray *ca, int idx, int idx1, int idx2, bool inverse);
	void oneDFftV(ComplexArray *ca, int idx, int idx1, int idx2, bool inverse);

	ComplexArray *mCA;
	QSize mSize;

};

#endif // FFT_H
