#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

class SplashWidget : public QWidget {
    Q_OBJECT
public:
    explicit SplashWidget(QWidget* parent = nullptr);

    // 配置：轮播图片、轮播间隔、总显示时长
    void setSlides(const QStringList& resPaths);
    void setSlideIntervalMs(int ms) { m_slideIntervalMs = ms; }
    void setTotalDurationMs(int ms) { m_totalDurationMs = ms; }

    // 开始播放 + 倒计时；到时自动淡出，并发出 finished() 信号
    void start();

signals:
    void finished();  // 淡出结束

private slots:
    void nextSlide();
    void beginFadeOut();
    void fadeFinished();

private:
    QLabel* m_label {nullptr};
    QVector<QPixmap> m_pix;
    int m_index {0};
    int m_slideIntervalMs {250};    // 每张切换间隔
    int m_totalDurationMs {3000};   // 总显示时长
    QTimer m_slideTimer;
    QTimer m_totalTimer;
    QGraphicsOpacityEffect* m_opacity {nullptr};
    QPropertyAnimation* m_anim {nullptr};
};
