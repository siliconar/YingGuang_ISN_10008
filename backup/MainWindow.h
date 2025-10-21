#pragma once
#include <QMainWindow>
#include <QProcess>
#include <QPointer>
#include <QVector>

class QAction;
class QToolBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* e) override;

private slots:
    void onLaunch1();
    void onLaunch2();
    void onLaunch3();
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
    QToolBar* m_toolbar {nullptr};
    QAction* m_act1 {nullptr};
    QAction* m_act2 {nullptr};
    QAction* m_act3 {nullptr};

    // 你自己的第三方程序路径（改成实际路径）
    QString m_exe1 = R"(apps\Internal1-Sun\Internal1-Sun.exe)";
    QString m_exe2 = R"(C:\Path\To\AppB.exe)";
    QString m_exe3 = R"(C:\Path\To\AppC.exe)";

    // 进程对象（非 startDetached，这样可以被我们控制/kill）
    QPointer<QProcess> m_p1;
    QPointer<QProcess> m_p2;
    QPointer<QProcess> m_p3;
};
