#ifndef SKELETONLOADER_H
#define SKELETONLOADER_H

#include <QWidget>
#include <QPropertyAnimation>

class SkeletonLoader : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal gradientPosition READ gradientPosition WRITE setGradientPosition)

public:
    explicit SkeletonLoader(QWidget *parent = nullptr);
    void demarrerAnimation();
    void arreterAnimation();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal gradientPosition() const;
    void setGradientPosition(qreal position);

    QPropertyAnimation *m_animation;
    qreal m_gradientPosition;
};

#endif // SKELETONLOADER_H
