#include "skeletonloader.h"
#include <QPainter>
#include <QLinearGradient>

SkeletonLoader::SkeletonLoader(QWidget *parent)
    : QWidget(parent),
    m_gradientPosition(0.0),
    m_darkMode(false),
    m_highlightColor(180, 220, 255) // Bleu clair pâle par défaut
{
    setFixedHeight(4);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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

void SkeletonLoader::setTheme(bool darkMode)
{
    m_darkMode = darkMode;
    update();
}

void SkeletonLoader::setHighlightColor(const QColor& color)
{
    m_highlightColor = color;
    update();
}

void SkeletonLoader::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Couleurs adaptatives
    QColor baseColor = m_darkMode ? QColor(60, 60, 60) : QColor(240, 240, 240);
    QColor highlight = m_highlightColor;

    // Ajustement automatique pour le mode sombre
    if(m_darkMode) {
        highlight = m_highlightColor.darker(120); // Assombrir légèrement en mode sombre
    }

    painter.fillRect(rect(), baseColor);

    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(qMax(0.0, m_gradientPosition - 0.3), baseColor.darker(110));
    gradient.setColorAt(m_gradientPosition, highlight);
    gradient.setColorAt(qMin(1.0, m_gradientPosition + 0.3), baseColor.darker(110));

    painter.fillRect(rect(), gradient);
}

qreal SkeletonLoader::gradientPosition() const {
    return m_gradientPosition;
}

void SkeletonLoader::setGradientPosition(qreal position) {
    m_gradientPosition = position;
    update();
}
