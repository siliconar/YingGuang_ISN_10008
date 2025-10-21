#include <QApplication>
#include "SplashWidget.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 1) 预加载页面
    SplashWidget splash;
    splash.setSlides({":/images/1.png", ":/images/2.png", ":/images/3.png"});
    splash.setSlideIntervalMs(250);  // 轮播间隔
    splash.setTotalDurationMs(1000); // 总显示 3 秒
    splash.start();

    // 2) 主窗口
    MainWindow w;

    QObject::connect(&splash, &SplashWidget::finished, [&](){
        w.show();
        w.raise();
        w.activateWindow();
    });

    return app.exec();
}
