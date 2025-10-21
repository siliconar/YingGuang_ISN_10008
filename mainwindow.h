#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QPointer>
#include <QVector>
#include "DbManagerWindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    protected:
    void closeEvent(QCloseEvent* e) override;

    private slots:
    void onLaunch1();
    void onLaunch2_1();
    void onLaunch2_2();
    void onLaunch3_1();
    void onLaunch3_2();
    void onLaunch3_3();
    void onLaunch3_4();
    void onLaunch3_5();
    void onAnyProcessStarted();
    void onAnyProcessFinished(int exitCode, QProcess::ExitStatus status);

    private:
    void setupUi();
    void setupProcesses();
    void bindProcess(QProcess* p);
    void updateUi();
    bool isAnyRunning() const;
    bool isProcessRunningSystemWide(const QString& exeBaseName) const; // Windows
    bool startProcessGuarded(QProcess* proc, const QString& exePath);

    void forceKillAllTargets();  // 退出时强杀（系统范围）
    void killOwnedProcesses();   // 只杀本程序启动的

private:
    // 你自己的第三方程序路径（改成实际路径）
    QString m_exe1 = R"(apps\Internal1-Sun\Internal1-Sun.exe)";
    QString m_exe2_1 = R"(C:\Path\To\AppB.exe)";
    QString m_exe2_2 = R"(apps\50-exe2_2\sif_retrieval_denoise_withGUI.exe)";
    QString m_exe3_1 = R"(C:\Path\To\AppC.exe)";
    QString m_exe3_2 = R"(C:\Path\To\AppC.exe)";
    QString m_exe3_3 = R"(apps\50-exe3_3\forest_gpp_estimator_gui.exe)";
    QString m_exe3_4 = R"(apps\50-exe3_4\grassland_gpp_estimator_gui.exe)";
    QString m_exe3_5 = R"(C:\Path\To\AppC.exe)";

    // 进程对象（非 startDetached，这样可以被我们控制/kill）
    QPointer<QProcess> m_p1;
    QPointer<QProcess> m_p2_1;
    QPointer<QProcess> m_p2_2;
    QPointer<QProcess> m_p3_1;
    QPointer<QProcess> m_p3_2;
    QPointer<QProcess> m_p3_3;
    QPointer<QProcess> m_p3_4;
    QPointer<QProcess> m_p3_5;


private:
    Ui::MainWindow *ui;

    DbManagerWindow *dia_db = nullptr;        // 设置器
};

#endif // MAINWINDOW_H
