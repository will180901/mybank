#include "skeletonloader.h"
#include <QPainter>
#include <QLinearGradient>

SkeletonLoader::SkeletonLoader(QWidget *parent)
    : QWidget(parent), m_gradientPosition(0.0)
{
    setMinimumHeight(2);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_animation = new QPropertyAnimation(this, "gradientPosition", this);
    m_animation->setDuration(1500);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setLoopCount(-1);
}

void SkeletonLoader::demarrerAnimation()
{
    show();
    m_animation->start();
}

void SkeletonLoader::arreterAnimation()
{
    m_animation->stop();
    hide();
}

void SkeletonLoader::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Fond gris de base
    painter.fillRect(rect(), QColor(240, 240, 240));

    // Dégradé avec meilleur contraste
    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(qMax(0.0, m_gradientPosition - 0.3), QColor(230, 230, 230));
    gradient.setColorAt(m_gradientPosition, QColor(200, 200, 200));
    gradient.setColorAt(qMin(1.0, m_gradientPosition + 0.3), QColor(230, 230, 230));

    painter.fillRect(rect(), gradient);
}

qreal SkeletonLoader::gradientPosition() const {
    return m_gradientPosition;
}

void SkeletonLoader::setGradientPosition(qreal position) {
    m_gradientPosition = position;
    update();
}
