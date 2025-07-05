#ifndef SKELETONLOADER_H
#define SKELETONLOADER_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QColor>

class SkeletonLoader : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal gradientPosition READ gradientPosition WRITE setGradientPosition)

public:
    explicit SkeletonLoader(QWidget *parent = nullptr);
    void demarrerAnimation();
    void arreterAnimation();
    void setTheme(bool darkMode);
    void setHighlightColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal gradientPosition() const;
    void setGradientPosition(qreal position);

    QPropertyAnimation *m_animation;
    qreal m_gradientPosition;
    bool m_darkMode;
    QColor m_highlightColor;
};

#endif // SKELETONLOADER_H
