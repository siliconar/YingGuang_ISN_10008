#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QProcess>
#include <QFileInfo>
#include <QTimer>
#include <QDateTime>
#include <QOperatingSystemVersion>

class PipelineUI : public QWidget {
    Q_OBJECT
public:
    PipelineUI(QWidget* parent=nullptr) : QWidget(parent) {
        setWindowTitle("MATLAB + ABC Pipeline");
        resize(600, 420);

        startBtn = new QPushButton("启动", this);
        stopBtn  = new QPushButton("停止", this);
        stopBtn->setEnabled(false);

        logEdit  = new QTextEdit(this);
        logEdit->setReadOnly(true);

        auto *lay = new QVBoxLayout(this);
        lay->addWidget(startBtn);
        lay->addWidget(stopBtn);
        lay->addWidget(logEdit);
        setLayout(lay);

        // ====== 配置区（按需修改）======
        // MATLAB 执行命令（注意：如果 matlab.exe 不在 PATH，就写绝对路径）
        matlabCmd = R"(matlab)";
        matlabArgs = {
            "-batch",
            R"(run('C:\Users\SITP\Desktop\sun01\荧光临时\test0.m');)"
        };

        // 期望 MATLAB 脚本生成的文件（示例）
        expectedFiles = {
            R"(C:\Users\SITP\Desktop\sun01\荧光临时\out\result1.txt)",
            R"(C:\Users\SITP\Desktop\sun01\荧光临时\out\result2.bin)"
        };

        // ABC 可执行程序（如果需要工作目录，也可设置 abcProc->setWorkingDirectory(...)）
        abcPath = R"(C:\Users\SITP\Desktop\sun01\荧光临时\ABC.exe)";
        // =================================

        connect(startBtn, &QPushButton::clicked, this, &PipelineUI::onStart);
        connect(stopBtn,  &QPushButton::clicked, this, &PipelineUI::onStop);
    }

private slots:
    void onStart() {
        if (state != Idle) return;

        stopRequested = false;
        startBtn->setEnabled(false);
        stopBtn->setEnabled(true);
        log("==== 启动流程 ====");

        // Step 1: 启动 MATLAB
        startMatlab();
    }

    void onStop() {
        if (state == Idle) return;

        stopRequested = true;
        log("收到停止指令，正在强制中止...");
        // 先尝试温和终止，再 taskkill 兜底
        killAllChildren();
        finishWithReset(false);
    }

    // MATLAB 结束时回调
    void onMatlabFinished(int exitCode, QProcess::ExitStatus status) {
        matlabPid = 0;
        if (stopRequested) return; // 已经在 stop 流程中做了收尾

        if (status != QProcess::NormalExit || exitCode != 0) {
            log(QString("MATLAB 运行失败，exitCode=%1").arg(exitCode));
            finishWithReset(false);
            return;
        }
        log("MATLAB 运行完成，开始检查结果文件...");
        state = CheckingFiles;

        // Step 2: 检查文件
        if (!checkExpectedFiles()) {
            log("结果文件不齐全，流程中止。");
            finishWithReset(false);
            return;
        }

        // Step 3: 运行 ABC.exe
        startAbc();
    }

    // ABC 结束时回调
    void onAbcFinished(int exitCode, QProcess::ExitStatus status) {
        abcPid = 0;
        if (stopRequested) return;

        if (status != QProcess::NormalExit || exitCode != 0) {
            log(QString("ABC.exe 运行失败，exitCode=%1").arg(exitCode));
            finishWithReset(false);
            return;
        }
        log("ABC.exe 运行完成。");
        finishWithReset(true); // Step 4: UI 复位（停止禁用 / 启动可用）
    }

    // 过程输出转发到日志
    void onProcStdOut() {
        auto *p = qobject_cast<QProcess*>(sender());
        if (!p) return;
        log(QString::fromLocal8Bit(p->readAllStandardOutput()));
    }
    void onProcStdErr() {
        auto *p = qobject_cast<QProcess*>(sender());
        if (!p) return;
        log(QString::fromLocal8Bit(p->readAllStandardError()));
    }

private:
    enum RunState { Idle, RunningMatlab, CheckingFiles, RunningAbc } state = Idle;

    QPushButton *startBtn{};
    QPushButton *stopBtn{};
    QTextEdit   *logEdit{};

    // 配置
    QString matlabCmd;
    QStringList matlabArgs;
    QStringList expectedFiles;
    QString abcPath;

    // 进程与状态
    QScopedPointer<QProcess> matlabProc;
    QScopedPointer<QProcess> abcProc;
    qint64 matlabPid = 0;
    qint64 abcPid = 0;
    bool stopRequested = false;

    void startMatlab() {
        state = RunningMatlab;
        matlabProc.reset(new QProcess(this));

        // 更干净：不弹 GUI（如需，可加 -nojvm -nodesktop -nosplash；但 -batch 已隐含自动退出）
        // 若想完全静默，可换成：
        // matlabArgs = {"-nojvm", "-nodesktop", "-nosplash", "-r", R"(run('...');exit;)"};

        connect(matlabProc.data(), &QProcess::readyReadStandardOutput, this, &PipelineUI::onProcStdOut);
        connect(matlabProc.data(), &QProcess::readyReadStandardError,  this, &PipelineUI::onProcStdErr);
        connect(matlabProc.data(), QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
                this, &PipelineUI::onMatlabFinished);

        log("启动 MATLAB...");
        matlabProc->start(matlabCmd, matlabArgs);
        if (!matlabProc->waitForStarted(10000)) {
            log("MATLAB 启动失败（10s 超时）。");
            finishWithReset(false);
            return;
        }
        matlabPid = matlabProc->processId();
        log(QString("MATLAB PID = %1").arg(matlabPid));
    }

    bool checkExpectedFiles() {
        for (const auto& path : expectedFiles) {
            QFileInfo fi(path);
            if (!fi.exists() || fi.size() <= 0) {
                log(QString("缺失文件：%1").arg(path));
                return false;
            }
            log(QString("已找到：%1  (%2 bytes)").arg(path).arg(fi.size()));
        }
        log("结果文件检查通过。");
        return true;
    }

    void startAbc() {
        state = RunningAbc;
        abcProc.reset(new QProcess(this));
        connect(abcProc.data(), &QProcess::readyReadStandardOutput, this, &PipelineUI::onProcStdOut);
        connect(abcProc.data(), &QProcess::readyReadStandardError,  this, &PipelineUI::onProcStdErr);
        connect(abcProc.data(), QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
                this, &PipelineUI::onAbcFinished);

        log(QString("启动 ABC.exe：%1").arg(abcPath));
        abcProc->start(abcPath);
        if (!abcProc->waitForStarted(10000)) {
            log("ABC.exe 启动失败（10s 超时）。");
            finishWithReset(false);
            return;
        }
        abcPid = abcProc->processId();
        log(QString("ABC PID = %1").arg(abcPid));
    }

    void finishWithReset(bool success) {
        // 结束所有子进程
        killAllChildren();

        state = Idle;
        stopRequested = false;
        stopBtn->setEnabled(false);
        startBtn->setEnabled(true);

        log(QString("==== 流程结束：%1 ====").arg(success ? "成功" : "失败/中止"));
    }

    void killAllChildren() {
        // 先优雅终止
        if (matlabProc) {
            matlabProc->terminate();
            matlabProc->waitForFinished(1500);
        }
        if (abcProc) {
            abcProc->terminate();
            abcProc->waitForFinished(1500);
        }

        // 兜底：强杀（含子进程树）
#ifdef Q_OS_WIN
        auto killPidTree = [this](qint64 pid) {
            if (pid <= 0) return;
            // /T 递归杀子进程，/F 强制
            QProcess::execute("taskkill", {"/PID", QString::number(pid), "/T", "/F"});
        };
        killPidTree(matlabPid);
        killPidTree(abcPid);

        // 有时 MATLAB 进程名是 matlab.exe，也可以按名字杀（当 PID 不可用时）
        // QProcess::execute("taskkill", {"/IM", "matlab.exe", "/T", "/F"});
#else
        // 非 Windows：用 kill -TERM / -KILL（按需实现）
        auto killPidTree = [](qint64 pid){
            if (pid > 0) {
                QProcess::execute("kill", {"-TERM", QString::number(pid)});
                QProcess::execute("kill", {"-KILL", QString::number(pid)});
            }
        };
        killPidTree(matlabPid);
        killPidTree(abcPid);
#endif

        // 清空句柄与 PID
        if (matlabProc) matlabProc->kill();
        if (abcProc)    abcProc->kill();
        matlabProc.reset(nullptr);
        abcProc.reset(nullptr);
        matlabPid = 0;
        abcPid = 0;
    }

    void log(const QString& s) {
        const QString line = QString("[%1] %2")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
            .arg(s.trimmed());
        logEdit->append(line);
    }
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PipelineUI w;
    w.show();
    return app.exec();
}
