/****************************************************************************
**
** Copyright (C) Qxt Foundation. Some rights reserved.
**
** This file is part of the QxtCore module of the Qt eXTension library
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** There is aditional information in the LICENSE file of libqxt.
** If you did not receive a copy of the file try to download it or
** contact the libqxt Management
** 
** <http://libqxt.sourceforge.net>  <aep@exys.org>  <coda@bobandgeorge.com>
**
****************************************************************************/


#include "QxtSpanSlider.h"
#include <QMouseEvent>
#include <QStylePainter>
#include <QStyleOptionSlider>

class QxtSpanSliderPrivate : public QxtPrivate<QxtSpanSlider>
{
public:
	QXT_DECLARE_PUBLIC(QxtSpanSlider);
	
	enum SpanHandle
	{
		LowerHandle,
		UpperHandle
	};
	
	void initStyleOption(QStyleOptionSlider* option, SpanHandle handle = UpperHandle) const;
	int pick(const QPoint& pt) const { return qxt_p().orientation() == Qt::Horizontal ? pt.x() : pt.y(); }
	int pixelPosToRangeValue(int pos) const;
	void handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, SpanHandle handle);
	void drawHandle(QStylePainter* painter, SpanHandle handle) const;
	void drawSpan(QStylePainter* painter, const QRect& rect) const;
	
	int lower;
	int upper;
	int offset;
	int position;
	SpanHandle lastPressed;
	QStyle::SubControl lowerPressed;
	QStyle::SubControl upperPressed;
};

// TODO: get rid of this in Qt 4.3
void QxtSpanSliderPrivate::initStyleOption(QStyleOptionSlider* option, SpanHandle handle) const
{
	if (!option)
		return;
	
	const QSlider* p = &qxt_p();
	option->initFrom(p);
	option->subControls = QStyle::SC_None;
	option->activeSubControls = QStyle::SC_None;
	option->orientation = p->orientation();
	option->maximum = p->maximum();
	option->minimum = p->minimum();
	option->tickPosition = p->tickPosition();
	option->tickInterval = p->tickInterval();
	option->upsideDown = (p->orientation() == Qt::Horizontal) ?
			(p->invertedAppearance() != (option->direction == Qt::RightToLeft)) : (!p->invertedAppearance());
	option->direction = Qt::LeftToRight; // we use the upsideDown option instead
	option->sliderPosition = (handle == LowerHandle ? lower : upper);
	option->sliderValue = (handle == LowerHandle ? lower : upper);
	option->singleStep = p->singleStep();
	option->pageStep = p->pageStep();
	if (p->orientation() == Qt::Horizontal)
		option->state |= QStyle::State_Horizontal;
}

int QxtSpanSliderPrivate::pixelPosToRangeValue(int pos) const
{
	QStyleOptionSlider opt;
	initStyleOption(&opt);
	
	int sliderMin = 0;
	int sliderMax = 0;
	int sliderLength = 0;
	const QSlider* p = &qxt_p();
	const QRect gr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
	const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
	if (p->orientation() == Qt::Horizontal)
	{
		sliderLength = sr.width();
		sliderMin = gr.x();
		sliderMax = gr.right() - sliderLength + 1;
	}
	else
	{
		sliderLength = sr.height();
		sliderMin = gr.y();
		sliderMax = gr.bottom() - sliderLength + 1;
	}
	return QStyle::sliderValueFromPosition(p->minimum(), p->maximum(), pos - sliderMin,
					       sliderMax - sliderMin, opt.upsideDown);
}

void QxtSpanSliderPrivate::handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, SpanHandle handle)
{
	QStyleOptionSlider opt;
	initStyleOption(&opt, handle);
	QSlider* p = &qxt_p();
	const QStyle::SubControl oldControl = control;
	control = p->style()->hitTestComplexControl(QStyle::CC_Slider, &opt, pos, p);
	const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
	if (control == QStyle::SC_SliderHandle)
	{
		position = value;
		offset = pick(pos - sr.topLeft());
		lastPressed = handle;
		p->setSliderDown(true);
	}
	if (control != oldControl)
		p->update(sr);
}

void QxtSpanSliderPrivate::drawSpan(QStylePainter* painter, const QRect& rect) const
{
	QStyleOptionSlider opt;
	initStyleOption(&opt);
	const QSlider* p = &qxt_p();
	QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
	
	if (opt.orientation == Qt::Horizontal)
		groove.adjust(0, 0, -1, 0);
	else
		groove.adjust(0, 0, 0, -1);
	
	// draw groove
	painter->setPen(QPen(p->palette().color(QPalette::Dark).light(110), 0));

	QColor activeHighlight = p->palette().color(QPalette::Active, QPalette::Highlight);
	QLinearGradient gradient;
	if (opt.orientation == Qt::Horizontal)
	{
		gradient.setStart(groove.center().x(), groove.top());
		gradient.setFinalStop(groove.center().x(), groove.bottom());
	}
	else
	{
		gradient.setStart(groove.left(), groove.center().y());
		gradient.setFinalStop(groove.right(), groove.center().y());
	}
	gradient.setColorAt(0, activeHighlight.dark(120));
	gradient.setColorAt(1, activeHighlight.light(108));
	painter->setBrush(gradient);
	
	if (opt.orientation == Qt::Horizontal)
	{
		painter->setPen(QPen(activeHighlight.dark(130), 0));
		painter->drawRect(rect.intersected(groove));
	}
	else
	{
		painter->setPen(QPen(activeHighlight.dark(150), 0));
		painter->drawRect(rect.intersected(groove));
	}
}

void QxtSpanSliderPrivate::drawHandle(QStylePainter* painter, SpanHandle handle) const
{
	QStyleOptionSlider opt;
	initStyleOption(&opt, handle);
	opt.subControls = QStyle::SC_SliderHandle;
	QStyle::SubControl pressed = (handle == LowerHandle ? lowerPressed : upperPressed);
	if (pressed == QStyle::SC_SliderHandle)
	{
		opt.activeSubControls = pressed;
		opt.state |= QStyle::State_Sunken;
	}
	painter->drawComplexControl(QStyle::CC_Slider, opt);
}

/*!
    \class QxtSpanSlider QxtSpanSlider
    \ingroup gui
    \brief A slider with two handles.

    QxtSpanSlider is a slider with two handles. QxtSpanSlider is
    handy for letting user to choose an span between min/max.

    \image html qxtspanslider.png "QxtSpanSlider in Plastique style."
 */

/*!
    \fn QxtSpanSlider::lowerChanged(int lower)

    This signal is emitted whenever the lower boundary has changed.
 */

/*!
    \fn QxtSpanSlider::upperChanged(int upper)

    This signal is emitted whenever the upper boundary has changed.
 */

/*!
    \fn QxtSpanSlider::spanChanged(int lower, int upper)

    This signal is emitted whenever the span has changed.
 */

/*!
    Constructs a new QxtSpanSlider with \a parent.
 */
QxtSpanSlider::QxtSpanSlider(QWidget* parent) : QSlider(parent)
{
	QXT_INIT_PRIVATE(QxtSpanSlider);
}

/*!
    Constructs a new QxtSpanSlider with \a orientation and \a parent.
 */
QxtSpanSlider::QxtSpanSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent)
{
	QXT_INIT_PRIVATE(QxtSpanSlider);
}

/*!
    Destructs the slider.
 */
QxtSpanSlider::~QxtSpanSlider()
{
}

/*!
    \property QxtLabel::lower
    \brief This property holds the lower boundary value of the span
 */
int QxtSpanSlider::lower() const
{
	return qMin(qxt_d().lower, qxt_d().upper);
}

void QxtSpanSlider::setLower(int lower)
{
	setSpan(lower, qxt_d().upper);
}

/*!
    \property QxtLabel::upper
    \brief This property holds the upper boundary value of the span
 */
int QxtSpanSlider::upper() const
{
	return qMax(qxt_d().lower, qxt_d().upper);
}

void QxtSpanSlider::setUpper(int upper)
{
	setSpan(qxt_d().lower, upper);
}

/*!
    Sets the span from \a lower to \a upper.
    \sa upper, lower
 */
void QxtSpanSlider::setSpan(int lower, int upper)
{
	const int l = qMin(lower, upper);
	const int u = qMax(lower, upper);
	if (l != qxt_d().lower || u != qxt_d().upper)
	{
		if (l != qxt_d().lower)
		{
			qxt_d().lower = l;
			emit lowerChanged(l);
		}
		if (u != qxt_d().upper)
		{
			qxt_d().upper = u;
			emit upperChanged(u);
		}
		emit spanChanged(qxt_d().lower, qxt_d().upper);
		update();
	}
}

void QxtSpanSlider::mousePressEvent(QMouseEvent* event)
{
	if (minimum() == maximum() || (event->buttons() ^ event->button()))
	{
		event->ignore();
		return;
	}
	
	qxt_d().handleMousePress(event->pos(), qxt_d().lowerPressed, qxt_d().lower, QxtSpanSliderPrivate::LowerHandle);
	qxt_d().handleMousePress(event->pos(), qxt_d().upperPressed, qxt_d().upper, QxtSpanSliderPrivate::UpperHandle);
	
	event->accept();
}

void QxtSpanSlider::mouseMoveEvent(QMouseEvent* event)
{
	if (qxt_d().lowerPressed != QStyle::SC_SliderHandle && qxt_d().upperPressed != QStyle::SC_SliderHandle)
	{
		event->ignore();
		return;
	}
	
	QStyleOptionSlider opt;
	qxt_d().initStyleOption(&opt);
	const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
	int newPosition = qxt_d().pixelPosToRangeValue(qxt_d().pick(event->pos()) - qxt_d().offset);
	if (m >= 0)
	{
		const QRect r = rect().adjusted(-m, -m, m, m);
		if (!r.contains(event->pos()))
		{
			newPosition = qxt_d().position;
		}
	}
	
	if (qxt_d().lowerPressed == QStyle::SC_SliderHandle)
	{
		if (newPosition > qxt_d().upper)
		{
			qSwap(qxt_d().lower, qxt_d().upper);
			qSwap(qxt_d().lowerPressed, qxt_d().upperPressed);
			setUpper(newPosition);
		}
		else
		{
			setLower(newPosition);
		}
	}
	else if (qxt_d().upperPressed == QStyle::SC_SliderHandle)
	{
		if (newPosition < qxt_d().lower)
		{
			qSwap(qxt_d().upper, qxt_d().lower);
			qSwap(qxt_d().upperPressed, qxt_d().lowerPressed);
			setLower(newPosition);
		}
		else
		{
			setUpper(newPosition);
		}
	}
	event->accept();
}

void QxtSpanSlider::mouseReleaseEvent(QMouseEvent* event)
{
	QSlider::mouseReleaseEvent(event);
	qxt_d().lowerPressed = QStyle::SC_None;
	qxt_d().upperPressed = QStyle::SC_None;
}

void QxtSpanSlider::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStylePainter painter(this);
	
	// ticks
	QStyleOptionSlider opt;
	qxt_d().initStyleOption(&opt);
	opt.subControls = QStyle::SC_SliderTickmarks;
	painter.drawComplexControl(QStyle::CC_Slider, opt);
	
	// groove
	opt.sliderPosition = 0;
	opt.subControls = QStyle::SC_SliderGroove;
	painter.drawComplexControl(QStyle::CC_Slider, opt);
	
	// handle rects
	opt.sliderPosition = qxt_d().lower;
	const QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	const int lrv  = qxt_d().pick(lr.center());
	opt.sliderPosition = qxt_d().upper;
	const QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	const int urv  = qxt_d().pick(ur.center());
	
	// span
	const int minv = qMin(lrv, urv);
	const int maxv = qMax(lrv, urv);
	const QPoint c = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this).center();
	QRect spanRect;
	if (orientation() == Qt::Horizontal)
		spanRect = QRect(QPoint(minv, c.y()-2), QPoint(maxv, c.y()+1));
	else
		spanRect = QRect(QPoint(c.x()-2, minv), QPoint(c.x()+1, maxv));
	qxt_d().drawSpan(&painter, spanRect);
	
	// handles
	switch (qxt_d().lastPressed)
	{
		case QxtSpanSliderPrivate::LowerHandle:
			qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::UpperHandle);
			qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::LowerHandle);
			break;
		case QxtSpanSliderPrivate::UpperHandle:
		default:
			qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::LowerHandle);
			qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::UpperHandle);
			break;
	}
}
