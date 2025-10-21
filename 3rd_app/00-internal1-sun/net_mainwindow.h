#ifndef NET_MAINWINDOW_H
#define NET_MAINWINDOW_H

#include <QDialog>
#include <QProcess>
#include <QPointer>
#include "pack_definition.h"

namespace Ui {
class net_mainwindow;
}

class net_mainwindow : public QDialog
{
    Q_OBJECT

public:
    explicit net_mainwindow(QWidget *parent = nullptr);
    ~net_mainwindow();

public: 
   void setUI();
   void setConnection();
    void setProcess();

private:
    Ui::net_mainwindow *ui;
    // QString _matlab_exe_path = "matlab.exe";
    // QString _matlab_script_path = R"(C:\Users\SITP\Desktop\sun01\荧光临时\test0.m)";


    QPointer<QProcess> m_p1_matlab;
    QPointer<QProcess> m_p2_pos_zx;
    QPointer<QProcess> m_p3_jihe;
    QPointer<QProcess> m_q1_FanYan;
    QPointer<QProcess> m_r2_YingYong;

    // 输出结果
    DataPack_matlab  datapack_matlab1;
    DataPack_pos_zx datapack_pos_zx1;
    DataPack_Jihe datapack_jihe1;

    void switchUI(int id);  // 切换UI

    // 进程相关
    void onStartProcess();
    void onStopProcess();
    void killOwnedProcesses();

    void onMatlabFinished(int, QProcess::ExitStatus);
    void onPos_zs_Finished(int, QProcess::ExitStatus);
    void onJihe_Finished(int, QProcess::ExitStatus);

    // matlab脚本
    void writeMatlabScript(const QString &scriptPath,
        const QString &dataPath,
        const QString &param);

};

#endif // NET_MAINWINDOW_H
