#include "qxtmultispanslider.h"
/****************************************************************************
**
*****************************************************************************/

#include "qxtmultispanslider_p.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QStyleOptionSlider>

QxtMultispanSliderPrivate::QxtMultispanSliderPrivate() :
        lower(0),
        upper(0),
        lowerPos(0),
        upperPos(0),
        offset(0),
        position(0),
        lastPressed(QxtMultispanSlider::NoHandle),
        mainControl(QxtMultispanSlider::LowerHandle),
        lowerPressed(QStyle::SC_None),
        upperPressed(QStyle::SC_None),
        movement(QxtMultispanSlider::FreeMovement),
        firstMovement(false),
        blockTracking(false)
{
}

void QxtMultispanSliderPrivate::initStyleOption(QStyleOptionSlider* option, QxtMultispanSlider::SpanHandle handle) const
{
    const QxtMultispanSlider* p = &qxt_p();
    p->initStyleOption(option);
    option->sliderPosition = (handle == QxtMultispanSlider::LowerHandle ? lowerPos : upperPos);
    option->sliderValue = (handle == QxtMultispanSlider::LowerHandle ? lower : upper);
}

int QxtMultispanSliderPrivate::pixelPosToRangeValue(int pos) const
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

void QxtMultispanSliderPrivate::handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, QxtMultispanSlider::SpanHandle handle)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    QxtMultispanSlider* p = &qxt_p();
    const QStyle::SubControl oldControl = control;
    control = p->style()->hitTestComplexControl(QStyle::CC_Slider, &opt, pos, p);
    const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
    if (control == QStyle::SC_SliderHandle)
    {
        position = value;
        offset = pick(pos - sr.topLeft());
        lastPressed = handle;
        p->setSliderDown(true);
        emit p->sliderPressed(handle);
    }
    if (control != oldControl)
        p->update(sr);
}

void QxtMultispanSliderPrivate::setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const
{
    QColor highlight = qxt_p().palette().color(QPalette::Highlight);
    QLinearGradient gradient(x1, y1, x2, y2);
    gradient.setColorAt(0, highlight.dark(120));
    gradient.setColorAt(1, highlight.light(108));
    painter->setBrush(gradient);

    if (orientation == Qt::Horizontal)
        painter->setPen(QPen(highlight.dark(130), 0));
    else
        painter->setPen(QPen(highlight.dark(150), 0));
}

void QxtMultispanSliderPrivate::drawSpan(QStylePainter* painter, const QRect& rect) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    const QSlider* p = &qxt_p();

    // area
    QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
    if (opt.orientation == Qt::Horizontal)
        groove.adjust(0, 0, -1, 0);
    else
        groove.adjust(0, 0, 0, -1);

    // pen & brush
    painter->setPen(QPen(p->palette().color(QPalette::Dark).light(110), 0));
    if (opt.orientation == Qt::Horizontal)
        setupPainter(painter, opt.orientation, groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
    else
        setupPainter(painter, opt.orientation, groove.left(), groove.center().y(), groove.right(), groove.center().y());

    // draw groove
    painter->drawRect(rect.intersected(groove));
}

void QxtMultispanSliderPrivate::drawHandle(QStylePainter* painter, QxtMultispanSlider::SpanHandle handle) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    opt.subControls = QStyle::SC_SliderHandle;
    QStyle::SubControl pressed = (handle == QxtMultispanSlider::LowerHandle ? lowerPressed : upperPressed);
    if (pressed == QStyle::SC_SliderHandle)
    {
        opt.activeSubControls = pressed;
        opt.state |= QStyle::State_Sunken;
    }
    painter->drawComplexControl(QStyle::CC_Slider, opt);
}

void QxtMultispanSliderPrivate::triggerAction(QAbstractSlider::SliderAction action, bool main)
{
    int value = 0;
    bool no = false;
    bool up = false;
    const int min = qxt_p().minimum();
    const int max = qxt_p().maximum();
    const QxtMultispanSlider::SpanHandle altControl = (mainControl == QxtMultispanSlider::LowerHandle ? QxtMultispanSlider::UpperHandle : QxtMultispanSlider::LowerHandle);

    blockTracking = true;

    switch (action)
    {
    case QAbstractSlider::SliderSingleStepAdd:
        if ((main && mainControl == QxtMultispanSlider::UpperHandle) || (!main && altControl == QxtMultispanSlider::UpperHandle))
        {
            value = qBound(min, upper + qxt_p().singleStep(), max);
            up = true;
            break;
        }
        value = qBound(min, lower + qxt_p().singleStep(), max);
        break;
    case QAbstractSlider::SliderSingleStepSub:
        if ((main && mainControl == QxtMultispanSlider::UpperHandle) || (!main && altControl == QxtMultispanSlider::UpperHandle))
        {
            value = qBound(min, upper - qxt_p().singleStep(), max);
            up = true;
            break;
        }
        value = qBound(min, lower - qxt_p().singleStep(), max);
        break;
    case QAbstractSlider::SliderToMinimum:
        value = min;
        if ((main && mainControl == QxtMultispanSlider::UpperHandle) || (!main && altControl == QxtMultispanSlider::UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderToMaximum:
        value = max;
        if ((main && mainControl == QxtMultispanSlider::UpperHandle) || (!main && altControl == QxtMultispanSlider::UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderMove:
        if ((main && mainControl == QxtMultispanSlider::UpperHandle) || (!main && altControl == QxtMultispanSlider::UpperHandle))
            up = true;
    case QAbstractSlider::SliderNoAction:
        no = true;
        break;
    default:
        qWarning("QxtMultispanSliderPrivate::triggerAction: Unknown action");
        break;
    }

    if (!no && !up)
    {
        if (movement == QxtMultispanSlider::NoCrossing)
            value = qMin(value, upper);
        else if (movement == QxtMultispanSlider::NoOverlapping)
            value = qMin(value, upper - 1);

        if (movement == QxtMultispanSlider::FreeMovement && value > upper)
        {
            swapControls();
            qxt_p().setUpperPosition(value);
        }
        else
        {
            qxt_p().setLowerPosition(value);
        }
    }
    else if (!no)
    {
        if (movement == QxtMultispanSlider::NoCrossing)
            value = qMax(value, lower);
        else if (movement == QxtMultispanSlider::NoOverlapping)
            value = qMax(value, lower + 1);

        if (movement == QxtMultispanSlider::FreeMovement && value < lower)
        {
            swapControls();
            qxt_p().setLowerPosition(value);
        }
        else
        {
            qxt_p().setUpperPosition(value);
        }
    }

    blockTracking = false;
    qxt_p().setLowerValue(lowerPos);
    qxt_p().setUpperValue(upperPos);
}

void QxtMultispanSliderPrivate::swapControls()
{
    qSwap(lower, upper);
    qSwap(lowerPressed, upperPressed);
    lastPressed = (lastPressed == QxtMultispanSlider::LowerHandle ? QxtMultispanSlider::UpperHandle : QxtMultispanSlider::LowerHandle);
    mainControl = (mainControl == QxtMultispanSlider::LowerHandle ? QxtMultispanSlider::UpperHandle : QxtMultispanSlider::LowerHandle);
}

void QxtMultispanSliderPrivate::updateRange(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    // setSpan() takes care of keeping span in range
    qxt_p().setSpan(lower, upper);
}

void QxtMultispanSliderPrivate::movePressedHandle()
{
    switch (lastPressed)
    {
        case QxtMultispanSlider::LowerHandle:
            if (lowerPos != lower)
            {
                bool main = (mainControl == QxtMultispanSlider::LowerHandle);
                triggerAction(QAbstractSlider::SliderMove, main);
            }
            break;
        case QxtMultispanSlider::UpperHandle:
            if (upperPos != upper)
            {
                bool main = (mainControl == QxtMultispanSlider::UpperHandle);
                triggerAction(QAbstractSlider::SliderMove, main);
            }
            break;
        default:
            break;
    }
}

/*!
    \class QxtMultispanSlider
    \inmodule QxtWidgets
    \brief The QxtMultispanSlider widget is a QSlider with multiple handles
	describing spans of a region.

    QxtMultispanSlider was designed for editing multiple colour gradients, of
	the type used in the GIMP.

    The span color is calculated based on QPalette::Highlight.

    The keys are bound according to the following table:
    \table
    \header \o Orientation    \o Key           \o Handle
    \row    \o Qt::Horizontal \o Qt::Key_Left  \o lower
    \row    \o Qt::Horizontal \o Qt::Key_Right \o lower
    \row    \o Qt::Horizontal \o Qt::Key_Up    \o upper
    \row    \o Qt::Horizontal \o Qt::Key_Down  \o upper
    \row    \o Qt::Vertical   \o Qt::Key_Up    \o lower
    \row    \o Qt::Vertical   \o Qt::Key_Down  \o lower
    \row    \o Qt::Vertical   \o Qt::Key_Left  \o upper
    \row    \o Qt::Vertical   \o Qt::Key_Right \o upper
    \endtable

    Keys are bound by the time the slider is created. A key is bound
    to same handle for the lifetime of the slider. So even if the handle
    representation might change from lower to upper, the same key binding
    remains.

    \image qxtmultispanslider.png "QxtMultispanSlider in Plastique style."

    \bold {Note:} QxtMultispanSlider inherits QSlider for implementation specific
    reasons. Adjusting any single handle specific properties like
    \list
    \o QAbstractSlider::sliderPosition
    \o QAbstractSlider::value
    \endlist
    has no effect. However, all slider specific properties like
    \list
    \o QAbstractSlider::invertedAppearance
    \o QAbstractSlider::invertedControls
    \o QAbstractSlider::minimum
    \o QAbstractSlider::maximum
    \o QAbstractSlider::orientation
    \o QAbstractSlider::pageStep
    \o QAbstractSlider::singleStep
    \o QSlider::tickInterval
    \o QSlider::tickPosition
    \endlist
    are taken into consideration.
 */

/*!
    \enum QxtMultispanSlider::HandleMovementMode

    This enum describes the available handle movement modes.

    \value FreeMovement The handles can be moved freely.
    \value NoCrossing The handles cannot cross, but they can still overlap each other. The lower and upper values can be the same.
    \value NoOverlapping The handles cannot overlap each other. The lower and upper values cannot be the same.
 */

/*!
    \enum QxtMultispanSlider::SpanHandle

    This enum describes the available span handles.

    \omitvalue NoHandle \omit Internal only (for now). \endomit
    \value LowerHandle The lower boundary handle.
    \value UpperHandle The upper boundary handle.
 */

/*!
    \fn QxtMultispanSlider::lowerValueChanged(int lower)

    This signal is emitted whenever the \a lower value has changed.
 */

/*!
    \fn QxtMultispanSlider::upperValueChanged(int upper)

    This signal is emitted whenever the \a upper value has changed.
 */

/*!
    \fn QxtMultispanSlider::spanChanged(int lower, int upper)

    This signal is emitted whenever both the \a lower and the \a upper
    values have changed ie. the span has changed.
 */

/*!
    \fn QxtMultispanSlider::lowerPositionChanged(int lower)

    This signal is emitted whenever the \a lower position has changed.
 */

/*!
    \fn QxtMultispanSlider::upperPositionChanged(int upper)

    This signal is emitted whenever the \a upper position has changed.
 */

/*!
    \fn QxtMultispanSlider::sliderPressed(SpanHandle handle)

    This signal is emitted whenever the \a handle has been pressed.
 */

/*!
    Constructs a new QxtMultispanSlider with \a parent.
 */
QxtMultispanSlider::QxtMultispanSlider(QWidget* parent) : QSlider(parent)
{
    QXT_INIT_PRIVATE(QxtMultispanSlider);
    connect(this, SIGNAL(rangeChanged(int, int)), &qxt_d(), SLOT(updateRange(int, int)));
    connect(this, SIGNAL(sliderReleased()), &qxt_d(), SLOT(movePressedHandle()));
}

/*!
    Constructs a new QxtMultispanSlider with \a orientation and \a parent.
 */
QxtMultispanSlider::QxtMultispanSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent)
{
    QXT_INIT_PRIVATE(QxtMultispanSlider);
    connect(this, SIGNAL(rangeChanged(int, int)), &qxt_d(), SLOT(updateRange(int, int)));
    connect(this, SIGNAL(sliderReleased()), &qxt_d(), SLOT(movePressedHandle()));
}

/*!
    Destructs the span slider.
 */
QxtMultispanSlider::~QxtMultispanSlider()
{
}

/*!
    \property QxtMultispanSlider::handleMovementMode
    \brief the handle movement mode
 */
QxtMultispanSlider::HandleMovementMode QxtMultispanSlider::handleMovementMode() const
{
    return qxt_d().movement;
}

void QxtMultispanSlider::setHandleMovementMode(QxtMultispanSlider::HandleMovementMode mode)
{
    qxt_d().movement = mode;
}

/*!
    \property QxtMultispanSlider::lowerValue
    \brief the lower value of the span
 */
int QxtMultispanSlider::lowerValue() const
{
    return qMin(qxt_d().lower, qxt_d().upper);
}

void QxtMultispanSlider::setLowerValue(int lower)
{
    setSpan(lower, qxt_d().upper);
}

/*!
    \property QxtMultispanSlider::upperValue
    \brief the upper value of the span
 */
int QxtMultispanSlider::upperValue() const
{
    return qMax(qxt_d().lower, qxt_d().upper);
}

void QxtMultispanSlider::setUpperValue(int upper)
{
    setSpan(qxt_d().lower, upper);
}

/*!
    Sets the span from \a lower to \a upper.
 */
void QxtMultispanSlider::setSpan(int lower, int upper)
{
    const int low = qBound(minimum(), qMin(lower, upper), maximum());
    const int upp = qBound(minimum(), qMax(lower, upper), maximum());
    if (low != qxt_d().lower || upp != qxt_d().upper)
    {
        if (low != qxt_d().lower)
        {
            qxt_d().lower = low;
            qxt_d().lowerPos = low;
            emit lowerValueChanged(low);
        }
        if (upp != qxt_d().upper)
        {
            qxt_d().upper = upp;
            qxt_d().upperPos = upp;
            emit upperValueChanged(upp);
        }
        emit spanChanged(qxt_d().lower, qxt_d().upper);
        update();
    }
}

/*!
    \property QxtMultispanSlider::lowerPosition
    \brief the lower position of the span
 */
int QxtMultispanSlider::lowerPosition() const
{
    return qxt_d().lowerPos;
}

void QxtMultispanSlider::setLowerPosition(int lower)
{
    if (qxt_d().lowerPos != lower)
    {
        qxt_d().lowerPos = lower;
        if (!hasTracking())
            update();
        if (isSliderDown())
            emit lowerPositionChanged(lower);
        if (hasTracking() && !qxt_d().blockTracking)
        {
            bool main = (qxt_d().mainControl == QxtMultispanSlider::LowerHandle);
            qxt_d().triggerAction(SliderMove, main);
        }
    }
}

/*!
    \property QxtMultispanSlider::upperPosition
    \brief the upper position of the span
 */
int QxtMultispanSlider::upperPosition() const
{
    return qxt_d().upperPos;
}

void QxtMultispanSlider::setUpperPosition(int upper)
{
    if (qxt_d().upperPos != upper)
    {
        qxt_d().upperPos = upper;
        if (!hasTracking())
            update();
        if (isSliderDown())
            emit upperPositionChanged(upper);
        if (hasTracking() && !qxt_d().blockTracking)
        {
            bool main = (qxt_d().mainControl == QxtMultispanSlider::UpperHandle);
            qxt_d().triggerAction(SliderMove, main);
        }
    }
}

/*!
    \reimp
 */
void QxtMultispanSlider::keyPressEvent(QKeyEvent* event)
{
    QSlider::keyPressEvent(event);

    bool main = true;
    SliderAction action = SliderNoAction;
    switch (event->key())
    {
    case Qt::Key_Left:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Right:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Up:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Down:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Home:
        main   = (qxt_d().mainControl == QxtMultispanSlider::LowerHandle);
        action = SliderToMinimum;
        break;
    case Qt::Key_End:
        main   = (qxt_d().mainControl == QxtMultispanSlider::UpperHandle);
        action = SliderToMaximum;
        break;
    default:
        event->ignore();
        break;
    }

    if (action)
        qxt_d().triggerAction(action, main);
}

/*!
    \reimp
 */
void QxtMultispanSlider::mousePressEvent(QMouseEvent* event)
{
    if (minimum() == maximum() || (event->buttons() ^ event->button()))
    {
        event->ignore();
        return;
    }

    qxt_d().handleMousePress(event->pos(), qxt_d().upperPressed, qxt_d().upper, QxtMultispanSlider::UpperHandle);
    if (qxt_d().upperPressed != QStyle::SC_SliderHandle)
        qxt_d().handleMousePress(event->pos(), qxt_d().lowerPressed, qxt_d().lower, QxtMultispanSlider::LowerHandle);

    qxt_d().firstMovement = true;
    event->accept();
}

/*!
    \reimp
 */
void QxtMultispanSlider::mouseMoveEvent(QMouseEvent* event)
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

    // pick the preferred handle on the first movement
    if (qxt_d().firstMovement)
    {
        if (qxt_d().lower == qxt_d().upper)
        {
            if (newPosition < lowerValue())
            {
                qxt_d().swapControls();
                qxt_d().firstMovement = false;
            }
        }
        else
        {
            qxt_d().firstMovement = false;
        }
    }

    if (qxt_d().lowerPressed == QStyle::SC_SliderHandle)
    {
        if (qxt_d().movement == NoCrossing)
            newPosition = qMin(newPosition, upperValue());
        else if (qxt_d().movement == NoOverlapping)
            newPosition = qMin(newPosition, upperValue() - 1);

        if (qxt_d().movement == FreeMovement && newPosition > qxt_d().upper)
        {
            qxt_d().swapControls();
            setUpperPosition(newPosition);
        }
        else
        {
            setLowerPosition(newPosition);
        }
    }
    else if (qxt_d().upperPressed == QStyle::SC_SliderHandle)
    {
        if (qxt_d().movement == NoCrossing)
            newPosition = qMax(newPosition, lowerValue());
        else if (qxt_d().movement == NoOverlapping)
            newPosition = qMax(newPosition, lowerValue() + 1);

        if (qxt_d().movement == FreeMovement && newPosition < qxt_d().lower)
        {
            qxt_d().swapControls();
            setLowerPosition(newPosition);
        }
        else
        {
            setUpperPosition(newPosition);
        }
    }
    event->accept();
}

/*!
    \reimp
 */
void QxtMultispanSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QSlider::mouseReleaseEvent(event);
    setSliderDown(false);
    qxt_d().lowerPressed = QStyle::SC_None;
    qxt_d().upperPressed = QStyle::SC_None;
    update();
}

/*!
    \reimp
 */
void QxtMultispanSlider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);

    // groove & ticks
    QStyleOptionSlider opt;
    qxt_d().initStyleOption(&opt);
    opt.sliderValue = 0;
    opt.sliderPosition = 0;
    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // handle rects
    opt.sliderPosition = qxt_d().lowerPos;
    const QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int lrv  = qxt_d().pick(lr.center());
    opt.sliderPosition = qxt_d().upperPos;
    const QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int urv  = qxt_d().pick(ur.center());

    // span
    const int minv = qMin(lrv, urv);
    const int maxv = qMax(lrv, urv);
    const QPoint c = QRect(lr.center(), ur.center()).center();
    QRect spanRect;
    if (orientation() == Qt::Horizontal)
        spanRect = QRect(QPoint(minv, c.y() - 2), QPoint(maxv, c.y() + 1));
    else
        spanRect = QRect(QPoint(c.x() - 2, minv), QPoint(c.x() + 1, maxv));
    qxt_d().drawSpan(&painter, spanRect);

    // handles
    switch (qxt_d().lastPressed)
    {
    case QxtMultispanSlider::LowerHandle:
        qxt_d().drawHandle(&painter, QxtMultispanSlider::UpperHandle);
        qxt_d().drawHandle(&painter, QxtMultispanSlider::LowerHandle);
        break;
    case QxtMultispanSlider::UpperHandle:
    default:
        qxt_d().drawHandle(&painter, QxtMultispanSlider::LowerHandle);
        qxt_d().drawHandle(&painter, QxtMultispanSlider::UpperHandle);
        break;
    }
}

