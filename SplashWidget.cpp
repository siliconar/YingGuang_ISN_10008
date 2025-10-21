#include "SplashWidget.h"
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QStyle>

SplashWidget::SplashWidget(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setWindowFlag(Qt::SplashScreen, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // 居中显示 & 指定大小（可按需调整）
    QSize sz(640, 360);
    resize(sz);
    auto scr = QGuiApplication::primaryScreen()->geometry();
    move(scr.center() - rect().center());

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setScaledContents(true);
    m_label->setMinimumSize(sz);
    m_label->resize(sz);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_label);

    connect(&m_slideTimer, &QTimer::timeout, this, &SplashWidget::nextSlide);
    connect(&m_totalTimer, &QTimer::timeout, this, &SplashWidget::beginFadeOut);

    m_opacity = new QGraphicsOpacityEffect(this);
    m_opacity->setOpacity(1.0);
    setGraphicsEffect(m_opacity);

    m_anim = new QPropertyAnimation(m_opacity, "opacity", this);
    m_anim->setDuration(600);
    m_anim->setStartValue(1.0);
    m_anim->setEndValue(0.0);
    connect(m_anim, &QPropertyAnimation::finished, this, &SplashWidget::fadeFinished);
}

void SplashWidget::setSlides(const QStringList& resPaths) {
    m_pix.clear();
    m_pix.reserve(resPaths.size());
    for (const auto& p : resPaths) {
        QPixmap pm(p);
        if (!pm.isNull())
            m_pix.push_back(pm);
    }
    if (!m_pix.isEmpty())
        m_label->setPixmap(m_pix[0]);
}

void SplashWidget::start() {
    show();
    raise();
    activateWindow();
    if (m_pix.size() > 1) {
        m_slideTimer.start(m_slideIntervalMs);
    }
    m_totalTimer.setSingleShot(true);
    m_totalTimer.start(m_totalDurationMs);
}

void SplashWidget::nextSlide() {
    if (m_pix.isEmpty()) return;
    m_index = (m_index + 1) % m_pix.size();
    m_label->setPixmap(m_pix[m_index]);
}

void SplashWidget::beginFadeOut() {
    m_slideTimer.stop();
    m_anim->start();
}

void SplashWidget::fadeFinished() {
    hide();
    emit finished();
}
