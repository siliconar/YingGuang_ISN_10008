#include "mainwindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
#include <QProcess>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setupUi();
  setupProcesses();
  updateUi();
}

MainWindow::~MainWindow() {
  delete ui;

  // 析构时再保险地结束我们拥有的子进程
  killOwnedProcesses();
}


void MainWindow::setupUi() {
    // setWindowTitle(u"荧光数据处理验证集成平台系统"_qs);
    // resize(400, 600);


    dia_backdoor = new Dialog_BackDoor(this);


    dia_db = new DbManagerWindow(this);
    // dia_db->setAttribute(Qt::WA_DeleteOnClose, true);
    dia_db->resize(900, 600);

    connect(ui->btn1_matlab,&QPushButton::clicked, this,&MainWindow::onLaunch1);
    connect(ui->btn2_1,&QPushButton::clicked, this,&MainWindow::onLaunch2_1);
    connect(ui->btn2_2,&QPushButton::clicked, this,&MainWindow::onLaunch2_2);
    connect(ui->btn3_1,&QPushButton::clicked, this,&MainWindow::onLaunch3_1);
    connect(ui->btn3_2,&QPushButton::clicked, this,&MainWindow::onLaunch3_2);
    connect(ui->btn3_3,&QPushButton::clicked, this,&MainWindow::onLaunch3_3);
    connect(ui->btn3_4,&QPushButton::clicked, this,&MainWindow::onLaunch3_4);
    // connect(ui->btn3_5,&QPushButton::clicked, this,&MainWindow::onLaunch3_5);


    // 数据库管理
    connect(ui->btn_DataBase, &QPushButton::clicked, this, [this](){


        if (dia_db->isVisible()) {
            // 已经显示 -> 隐藏
            dia_db->close();
          } else {
            // 没显示 -> 打开
            dia_db->show();
            dia_db->raise();           // 保证弹出在前
            dia_db->activateWindow();  // 设为激活窗口
          }
    });

    // 后门
    connect(ui->btn_backdoor, &QPushButton::clicked, this, [this](){


        if (dia_backdoor->isVisible()) {
            // 已经显示 -> 隐藏
            dia_backdoor->close();
          } else {
            // 没显示 -> 打开
            dia_backdoor->show();
            dia_backdoor->raise();           // 保证弹出在前
            dia_backdoor->activateWindow();  // 设为激活窗口
          }
    });

    statusBar()->showMessage(u"就绪"_qs);
}

void MainWindow::setupProcesses() {
    m_p1 = new QProcess(this);
    m_p2_1 = new QProcess(this);
    m_p2_2 = new QProcess(this);
    m_p3_1 = new QProcess(this);
    m_p3_2 = new QProcess(this);
    m_p3_3 = new QProcess(this);
    m_p3_4 = new QProcess(this);
    m_p3_5 = new QProcess(this);

    bindProcess(m_p1);
    bindProcess(m_p2_1);
    bindProcess(m_p2_2);
    bindProcess(m_p3_1);
    bindProcess(m_p3_2);
    bindProcess(m_p3_3);
    bindProcess(m_p3_4);
    bindProcess(m_p3_5);
}

void MainWindow::onLaunch1() {
    startProcessGuarded(m_p1, m_exe1);
}
void MainWindow::onLaunch2_1() {
    startProcessGuarded(m_p2_1, m_exe2_1);
}
void MainWindow::onLaunch2_2() {
    startProcessGuarded(m_p2_2, m_exe2_2);
}
void MainWindow::onLaunch3_1() {
    startProcessGuarded(m_p3_1, m_exe3_1);
}
void MainWindow::onLaunch3_2() {

    // 从后门读
    m_exe3_2 = dia_backdoor->path3_2;

    startProcessGuarded(m_p3_2, m_exe3_2);
}
void MainWindow::onLaunch3_3() {
    startProcessGuarded(m_p3_3, m_exe3_3);
}
void MainWindow::onLaunch3_4() {
    startProcessGuarded(m_p3_4, m_exe3_4);
}
void MainWindow::onLaunch3_5() {
    startProcessGuarded(m_p3_5, m_exe3_5);
}



void MainWindow::bindProcess(QProcess* p) {
    connect(p, &QProcess::started, this, &MainWindow::onAnyProcessStarted);
    connect(p, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &MainWindow::onAnyProcessFinished);
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
    if (m_p2_1 && m_p2_1->state() != st) return true;
    if (m_p2_2 && m_p2_2->state() != st) return true;
    if (m_p3_1 && m_p3_1->state() != st) return true;
    if (m_p3_2 && m_p3_2->state() != st) return true;
    if (m_p3_3 && m_p3_3->state() != st) return true;
    if (m_p3_4 && m_p3_4->state() != st) return true;
    if (m_p3_5 && m_p3_5->state() != st) return true;

    return false;
}

void MainWindow::updateUi() {
    const bool busy = isAnyRunning();
    ui->btn1_matlab->setEnabled(!busy);
    ui->btn2_1->setEnabled(!busy);
    ui->btn2_2->setEnabled(!busy);
    ui->btn3_1->setEnabled(!busy);
    ui->btn3_2->setEnabled(!busy);
    ui->btn3_3->setEnabled(!busy);
    ui->btn3_4->setEnabled(!busy);
    // ui->btn3_5->setEnabled(!busy);
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
        killOne(m_p2_1);
        killOne(m_p2_2);
        killOne(m_p3_1);
        killOne(m_p3_2);
        killOne(m_p3_3);
        killOne(m_p3_4);
        killOne(m_p3_5);
    }
    
    void MainWindow::forceKillAllTargets() {
    #ifdef Q_OS_WIN
        auto killByName = [](const QString& exeName) {
            if (exeName.isEmpty()) return;
            QProcess::execute("taskkill", {"/IM", exeName, "/F"});
        };
        killByName(QFileInfo(m_exe1).fileName());
        killByName(QFileInfo(m_exe2_1).fileName());
        killByName(QFileInfo(m_exe2_2).fileName());
        killByName(QFileInfo(m_exe3_1).fileName());
        killByName(QFileInfo(m_exe3_2).fileName());
        killByName(QFileInfo(m_exe3_3).fileName());
        killByName(QFileInfo(m_exe3_4).fileName());
        killByName(QFileInfo(m_exe3_5).fileName());
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