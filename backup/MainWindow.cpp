#include "MainWindow.h"
#include <QToolBar>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QFileInfo>
#include <QCloseEvent>
#include <QDebug>

#ifdef Q_OS_WIN
  #include <QProcess>
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    setupProcesses();
    updateUi();
}

MainWindow::~MainWindow() {
    // 析构时再保险地结束我们拥有的子进程
    killOwnedProcesses();
}

void MainWindow::setupUi() {
    setWindowTitle(u"荧光数据处理验证集成平台系统"_qs);
    resize(400, 600);

    m_toolbar = addToolBar(u"工具栏"_qs);
    m_toolbar->setMovable(false);

    m_act1 = m_toolbar->addAction(u"1.预处理环节"_qs, this, &MainWindow::onLaunch1);
    m_act2 = m_toolbar->addAction(u"2.反演环节"_qs, this, &MainWindow::onLaunch2);
    m_act3 = m_toolbar->addAction(u"3.应用环节"_qs, this, &MainWindow::onLaunch3);

    statusBar()->showMessage(u"就绪"_qs);
}

void MainWindow::setupProcesses() {
    m_p1 = new QProcess(this);
    m_p2 = new QProcess(this);
    m_p3 = new QProcess(this);

    bindProcess(m_p1);
    bindProcess(m_p2);
    bindProcess(m_p3);
}

void MainWindow::bindProcess(QProcess* p) {
    connect(p, &QProcess::started, this, &MainWindow::onAnyProcessStarted);
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &MainWindow::onAnyProcessFinished);
}

void MainWindow::onLaunch1() {
    startProcessGuarded(m_p1, m_exe1);
}
void MainWindow::onLaunch2() {
    startProcessGuarded(m_p2, m_exe2);
}
void MainWindow::onLaunch3() {
    startProcessGuarded(m_p3, m_exe3);
}

bool MainWindow::startProcessGuarded(QProcess* proc, const QString& exePath) {
    QFileInfo fi(exePath);
    if (!fi.exists()) {
        QMessageBox::warning(this, u"启动失败"_qs, u"找不到可执行文件：\n"_qs + exePath);
        return false;
    }

    // 若系统中已在运行（无论谁启动），不再重复启动
    const QString base = fi.fileName(); // 如 "AppA.exe"
    if (isProcessRunningSystemWide(base)) {
        QMessageBox::information(this, u"已在运行"_qs,
                                 u"目标程序已在运行："_qs + base);
        return false;
    }

    // 若有任一在运行，禁止启动（全局互斥）
    if (isAnyRunning()) {
        QMessageBox::information(this, u"正在运行"_qs,
                                 u"已有任务在运行，请先结束后再启动。"_qs);
        return false;
    }

    proc->setProgram(exePath);
    proc->setArguments({}); // 如需参数，填在这里
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->start();

    if (!proc->waitForStarted(3000)) {
        QMessageBox::warning(this, u"启动失败"_qs,
                             u"进程启动超时/失败：\n"_qs + exePath + u"\n错误："_qs + proc->errorString());
        return false;
    }

    statusBar()->showMessage(u"运行中："_qs + base);
    updateUi();
    return true;
}

void MainWindow::onAnyProcessStarted() {
    updateUi();
}

void MainWindow::onAnyProcessFinished(int, QProcess::ExitStatus) {
    // 若都结束，恢复可点击
    updateUi();
    if (!isAnyRunning()) {
        statusBar()->showMessage(u"就绪"_qs, 2000);
    }
}

bool MainWindow::isAnyRunning() const {
    const auto st = QProcess::NotRunning;
    if (m_p1 && m_p1->state() != st) return true;
    if (m_p2 && m_p2->state() != st) return true;
    if (m_p3 && m_p3->state() != st) return true;
    return false;
}

void MainWindow::updateUi() {
    const bool busy = isAnyRunning();
    m_act1->setEnabled(!busy);
    m_act2->setEnabled(!busy);
    m_act3->setEnabled(!busy);
}

bool MainWindow::isProcessRunningSystemWide(const QString& exeBaseName) const {
#ifdef Q_OS_WIN
    // 调用 tasklist /FI "IMAGENAME eq xxx.exe"
    QProcess p;
    QStringList args = {"/FI", QString("IMAGENAME eq %1").arg(exeBaseName)};
    p.start("tasklist", args);
    if (!p.waitForFinished(3000)) return false;
    const QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
    // tasklist 输出包含进程名那一行，简单判断
    return out.contains(exeBaseName, Qt::CaseInsensitive);
#else
    Q_UNUSED(exeBaseName);
    // TODO: Linux/Mac 可用 pgrep -x，或 /proc 遍历；此处返回 false 作为占位
    return false;
#endif
}

void MainWindow::killOwnedProcesses() {
    auto killOne = [](QPointer<QProcess> p) {
        if (!p) return;
        if (p->state() != QProcess::NotRunning) {
            p->kill();
            p->waitForFinished(2000);
        }
    };
    killOne(m_p1);
    killOne(m_p2);
    killOne(m_p3);
}

void MainWindow::forceKillAllTargets() {
#ifdef Q_OS_WIN
    auto killByName = [](const QString& exeName) {
        if (exeName.isEmpty()) return;
        QProcess::execute("taskkill", {"/IM", exeName, "/F"});
    };
    killByName(QFileInfo(m_exe1).fileName());
    killByName(QFileInfo(m_exe2).fileName());
    killByName(QFileInfo(m_exe3).fileName());
#else
    // Linux/mac：可改用 pkill -9 <name> 或自建 PID 文件管理
#endif
}

void MainWindow::closeEvent(QCloseEvent* e) {
    // 先杀我们拥有的，再系统范围兜底强杀
    killOwnedProcesses();
    forceKillAllTargets();
    e->accept();
}
