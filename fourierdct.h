#ifndef FOURIERDCT_H
#define FOURIERDCT_H

#include "imagetransformfilter.h"

class FourierDCT : public ImageTransformFilter
{
	Q_OBJECT
public:
	explicit FourierDCT(QObject *parent = 0);
	~FourierDCT();
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
	void rearrangeDct(QVector<Complex> &elements);
	void transform(QVector<Complex> &elements, bool inverse);
	void oneDFftH(ComplexArray *ca, int idx, int idx1, int idx2, bool inverse);
	void oneDFftV(ComplexArray *ca, int idx, int idx1, int idx2, bool inverse);
	void prepareScale(int n);
	void prepareFft(ComplexArray *ca, int idx, int idx1, int idx2);
	void test();
	qreal alpha(int u) const;

	ComplexArray *mCA;
	QSize mSize;
	QVector<Complex> mW;
	QVector<Complex> mScale;
	qreal mAlphaAC, mAlphaDC;

};

#endif // FOURIERDCT_H
