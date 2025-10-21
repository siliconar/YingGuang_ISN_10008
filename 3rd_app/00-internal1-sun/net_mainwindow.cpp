#include "net_mainwindow.h"
#include "ui_net_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>
#include "ConfigManager.h"

#include <QTextCodec>
#include <QFile>
#include <QStringConverter>   // 关键：Qt6 的编码枚举在这里

net_mainwindow::net_mainwindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::net_mainwindow)
{
    ui->setupUi(this);

    setUI();
    setConnection();

    setProcess();
}

net_mainwindow::~net_mainwindow()
{
    delete ui;
}

void net_mainwindow::setUI()
{
    // QString img1 = "C:/Users/SITP/Desktop/sun01/YingGuang_ISN_10008/3rd_app/00-internal1-sun/images/LCVNIR-20250613_145257-00001.png";
    // QPixmap pm(img1);
    // ui->label_res->setScaledContents(true);
    // ui->label_res->setPixmap(pm);

    ui->combo_integ_time->addItem("18ms");
    ui->combo_integ_time->addItem("48ms");


  // 读取配置，设置一下脚本根目录
    if (ConfigManager::instance().getValue("matlabSetting/ScriptRootPath").isValid())
      ui->text_ScriptRootPath->setText(ConfigManager::instance().getValue("matlabSetting/ScriptRootPath").toString());  // 脚本注入目录
}

void net_mainwindow::setConnection()
{
    // 选择文件

    connect(ui->btn_selectFile, &QPushButton::clicked, this, [this]() {
      // 先看看配置ini里有没有默认文件
      QString default_path1 = "C:/";
      // 读取配置
      if (ConfigManager::instance().getValue("matlabSetting/ReadFile_Path").isValid())
        default_path1 = ConfigManager::instance().getValue("matlabSetting/ReadFile_Path").toString();  // 脚本注入目录

      // 打开选择窗口
      QString filePath = QFileDialog::getOpenFileName(
          this, tr("选择文件"), default_path1, tr("所有文件 (*.*)"));
      if (!filePath.isEmpty()) {
        ui->text_file->setText(filePath);
        // 写入本地配置
                  ConfigManager::instance().setValue("matlabSetting/ReadFile_Path",
            QFileInfo(filePath).path());  // 保存设置
      }
    });

    // 启动按钮
    connect(ui->btn_Start,&QPushButton::clicked,this,&net_mainwindow::onStartProcess);

    // 停止按钮
    connect(ui->btn_Stop,&QPushButton::clicked,this,&net_mainwindow::onStopProcess);

    // 设置Setting切换
    connect(ui->btn_Setting, &QPushButton::clicked, this, [this]() {
      if (ui->stackedWidget->currentIndex() == 0) {
        ui->stackedWidget->setCurrentIndex(1);

          // 读取配置
        if (ConfigManager::instance()
                .getValue("matlabSetting/ScriptRootPath")
                .isValid())
          ui->text_ScriptRootPath->setText(
              ConfigManager::instance()
                  .getValue("matlabSetting/ScriptRootPath")
                  .toString());   // 脚本注入目录

      } else {
        ui->stackedWidget->setCurrentIndex(0);

          // 写入配置
          ConfigManager::instance().setValue("matlabSetting/ScriptRootPath",
            ui->text_ScriptRootPath->text());  // 保存设置
      }
    });
}


void net_mainwindow::switchUI(int id)  // 切换UI
{
    // 按下End，或者初始界面
    if(0==id)
    {
        ui->btn_Start->setEnabled(true);
        ui->btn_Stop->setEnabled(false);
        ui->text_file->setEnabled(true);
        ui->combo_integ_time->setEnabled(true);
        ui->progressBar->setValue(0);

        ui->label_Status->setText("就绪");
    }
    else if(1==id)  // 按下Start
    {
        ui->btn_Start->setEnabled(false);
        ui->btn_Stop->setEnabled(true);
        ui->text_file->setEnabled(false);
        ui->combo_integ_time->setEnabled(false);
    }


}

void net_mainwindow::setProcess() {
  m_p1_matlab = new QProcess(this);
  m_p2_pos_zx = new QProcess(this);
  m_p3_jihe = new QProcess(this);

  connect(m_p1_matlab,
          qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
          &net_mainwindow::onMatlabFinished);
  connect(m_p2_pos_zx,
          qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
          &net_mainwindow::onPos_zs_Finished);
  connect(m_p3_jihe, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
          this, &net_mainwindow::onJihe_Finished);

  // 查看输出
//   connect(m_p3_jihe, &QProcess::readyReadStandardOutput, this, [=]() {
//     QByteArray data = m_p3_jihe->readAllStandardOutput();
//     qDebug().noquote() << QString::fromUtf8(data);
//   });

connect(m_p3_jihe, &QProcess::readyReadStandardOutput, this, [=]() {
    QByteArray data = m_p3_jihe->readAllStandardOutput();
    QString text = QString::fromUtf8(data.constData(), data.size());

    // 检查是否有非法字符（替换符号 U+FFFD）
    if (text.contains(QChar(0xFFFD))) {
        qWarning() << "[Warning] 输出中包含无法解码的字符，已跳过显示。";
    } else {
        qDebug().noquote() << text;
    }
});



}

void net_mainwindow::onStartProcess()
{

    // 整理包，并检查文件
    datapack_matlab1.input_dat_full = ui->text_file->text();   // path/LCVNIR-20250613_144349-00001.dat
    QFileInfo info1(datapack_matlab1.input_dat_full);
    datapack_matlab1.base_name = info1.baseName();              // LCVNIR-20250613_144349-00001
    datapack_matlab1.integ_time = ui->combo_integ_time->currentText();  // 18ms
    QString input_dir = info1.path();   // 未完成路径确认，记得确认matlab输出路径是否正确
    datapack_matlab1.out_folder = QDir(input_dir).filePath(datapack_matlab1.base_name);  // path/LCVNIR-20250613_144349-00001/
    datapack_matlab1.out_png_name_full = QDir(datapack_matlab1.out_folder).filePath(datapack_matlab1.base_name+".png");  // outpath/LCVNIR-20250613_145257-00001.png
    datapack_matlab1.out_L1B_dat_name_full = QDir(datapack_matlab1.out_folder).filePath(datapack_matlab1.base_name+"_L1B.dat");  // outpath/LCVNIR-20250613_145257-00001_L1B.dat

    // 生成先导matlab脚本
    QString scriptfull = QDir(ui->text_ScriptRootPath->text()).filePath("test_1.m");
    writeMatlabScript(scriptfull, datapack_matlab1.input_dat_full,datapack_matlab1.integ_time);

    // 运行matlab脚本
   // MATLAB 执行命令（注意：如果 matlab.exe 不在 PATH，就写绝对路径）
    QString matlabCmd = R"(matlab)";
    // QStringList matlabArgs = {
    //     "-batch", 
    //     // QString(R"(run('%1');)").arg(scriptfull)
    //     R"(run('C:\Users\SITP\Desktop\sun01\荧光临时\test0.m');)"   // 未完成，最后得改
    
    // };

    QStringList matlabArgs = {
      "-automation", 
      "-nosplash",
      "-nodesktop",
      "-wait",
      "-r",
      QString(R"(run('%1');exit;)").arg(scriptfull)
      // R"(run('C:\Users\SITP\Desktop\sun01\荧光临时\test0.m');exit;)"   // 未完成，最后得改
  
  };

    
    qDebug()<<matlabCmd<<" "<<matlabArgs[0]<<" "<<matlabArgs[1];

    m_p1_matlab->start(matlabCmd, matlabArgs);    
    if (!m_p1_matlab->waitForStarted(5000)) {
        QMessageBox::warning(this, u"启动失败"_qs,
                             u"进程启动超时/失败：\n"_qs + matlabCmd + u"\n错误："_qs + m_p1_matlab->errorString());
        return;
    }

    // 关闭启动按钮，打开停止按钮
    switchUI(1);

    // 启动完毕
    ui->label_Status->setText("正在执行: 预处理步骤...");
    ui->progressBar->setValue(10);

    return;
}

void net_mainwindow::onMatlabFinished(int, QProcess::ExitStatus) {

  // 再次检查是否结束
  if (m_p1_matlab && m_p1_matlab->state() != QProcess::NotRunning) {
    qDebug() << "matlab进程未结束，本不应该来这里";
  }

  // 文件安全检查
  QFileInfo fi1(datapack_matlab1.out_L1B_dat_name_full+".txt");
  if (!fi1.exists()) {
    QMessageBox::warning(this, u"Matlab结果不存在"_qs,
                         u"文件: "_qs + datapack_matlab1.out_L1B_dat_name_full);
    switchUI(0);
    return;
  }

  // // // 未完成，最后删除，临时的文件安全检查
  // QFileInfo fi2(datapack_matlab1.out_L1B_dat_name_full+"123.txt");
  // if (!fi2.exists()) {
  //   QMessageBox::warning(this, u"Matlab结果不存在2"_qs,
  //                        u"文件: "_qs + datapack_matlab1.out_L1B_dat_name_full);
  //   switchUI(0);
  //   return;
  // }


  // 准备启动pos的数据
  datapack_pos_zx1.in_dat_full = datapack_matlab1.input_dat_full;
  QFileInfo info2(datapack_pos_zx1.in_dat_full);
  datapack_pos_zx1.out_csv_full = QDir(info2.absolutePath()).filePath(info2.baseName()) + "_POS.csv";

  // 准备启动POS的EXE
  const QString appDir = QCoreApplication::applicationDirPath();
  const QString exePath = QDir(appDir).filePath("apps/20pos-zx/POS_YG_Core.exe"); 
  m_p2_pos_zx->setProgram(exePath);  
  m_p2_pos_zx->setArguments({datapack_pos_zx1.in_dat_full, datapack_pos_zx1.out_csv_full}); // 如需参数，填在这里
  m_p2_pos_zx->setProcessChannelMode(QProcess::MergedChannels);
  m_p2_pos_zx->start();

  qDebug()<<datapack_pos_zx1.in_dat_full<<" "<<datapack_pos_zx1.out_csv_full;
   
  if (!m_p2_pos_zx->waitForStarted(5000)) {
      QMessageBox::warning(this, u"启动失败"_qs,
                           u"进程启动超时/失败：pos_zx.exe\n"_qs + u"\n错误："_qs + m_p2_pos_zx->errorString());

      switchUI(0);
      return;
  }


  // 启动完毕
  ui->label_Status->setText("正在执行: POS处理步骤...");
  ui->progressBar->setValue(40);
}

void net_mainwindow::onPos_zs_Finished(int, QProcess::ExitStatus) {

  // 再次检查是否结束
  if (m_p2_pos_zx && m_p2_pos_zx->state() != QProcess::NotRunning) {
    qDebug() << "pos_zx进程未结束，本不应该来这里";
  }

  // 再次检查文件是否生成
  QFileInfo fi1(datapack_pos_zx1.out_csv_full);
  if (!fi1.exists()) {
    QMessageBox::warning(this, u"POS zx结果不存在"_qs,
                         u"文件: "_qs + datapack_pos_zx1.out_csv_full);
    switchUI(0);
    return;
  }

  // 准备启动几何程序
  datapack_jihe1.in_dat_full = datapack_matlab1.out_L1B_dat_name_full;
  datapack_jihe1.in_csv_full = datapack_pos_zx1.out_csv_full;
  QFileInfo info_in(datapack_jihe1.in_dat_full);
  datapack_jihe1.basename1 = info_in.baseName();
  datapack_jihe1.out_folder = QDir(info_in.path()).filePath("out_jihe"); 
  datapack_jihe1.out_L_jpg_full = QDir(datapack_jihe1.out_folder).filePath(datapack_jihe1.basename1 + "_L_RGB.jpg");

  // 自动创建目录
  QDir dir22;
  dir22.mkpath(datapack_jihe1.out_folder);

  // 启动程序
  const QString appDir = QCoreApplication::applicationDirPath();
  // const QString exePath = QDir(appDir).filePath("apps/30jihe/bes.exe"); // 未完成，最后得改
    const QString exePath = QDir(appDir).filePath("apps/30jihe/LCVNIR_rotate.exe"); 


    QStringList exeArgs = {
        "--pos", datapack_jihe1.in_csv_full,
        "--img", datapack_jihe1.in_dat_full,
        "--out", datapack_jihe1.out_folder
    };
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // 关键：强制 Python 使用 UTF-8（3.7+）
    env.insert("PYTHONUTF8", "1");
    env.insert("PYTHONIOENCODING", "utf-8");              // 保险
    
    m_p3_jihe->setProcessEnvironment(env);
    m_p3_jihe->setWorkingDirectory(QFileInfo(exePath).absolutePath());
    m_p3_jihe->setProgram(exePath);
    m_p3_jihe->setArguments(exeArgs);
    m_p3_jihe->setProcessChannelMode(QProcess::MergedChannels);
    m_p3_jihe->start();


  // 启动完毕
  ui->label_Status->setText("正在执行: 几何处理步骤...");
  ui->progressBar->setValue(80);


}

void net_mainwindow::onJihe_Finished(int, QProcess::ExitStatus) {
  switchUI(0);

  // 再次检查文件是否生成
  QFileInfo fi1(datapack_jihe1.out_L_jpg_full);
  if (!fi1.exists()) {
    QMessageBox::warning(this, u"几何结果不存在"_qs,
                         u"文件: "_qs + datapack_jihe1.out_L_jpg_full);
    // switchUI(0);
    return;
  }

  // QString img1 =
  // "E:/Y.荧光/matlab处理/示例数据/48ms/LCVNIR-20250613_145257-00001/LCVNIR-20250613_145257-00001_L1B_L_RGB.jpg"; // 未完成，要改
  QString img1 = datapack_jihe1.out_L_jpg_full;
  QPixmap pm(img1);
  ui->label_res->setScaledContents(true);
  ui->label_res->setPixmap(pm);
}

void net_mainwindow::writeMatlabScript(const QString &scriptPath,
                                       const QString &dataPath,
                                       const QString &param) {
  // 拼出要写入的内容
  QString line = QString("product_L1data('%1','%2');").arg(dataPath).arg(param);
  const QString header = QStringLiteral("% 文件编码：GB18030（请勿另存为UTF-8）\r\n");

      // 3) 组合
      const QString content = header + line;

  QFile file(scriptPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << "无法创建脚本:" << scriptPath;
    return;
  }

  // GB18030（也可换成 "GBK"）
  QTextCodec *codec = QTextCodec::codecForName("GB18030");
  if (!codec) {
    qWarning() << "此构建不支持GB18030编码，改用UTF-8";
    file.write(content.toUtf8());
  } else {
    QByteArray bytes = codec->fromUnicode(content);
    file.write(bytes);
  }
  file.close();





  // QTextStream out(&file);
  // auto encOpt = QStringConverter::encodingForName("GB18030");
  // out.setEncoding(encOpt.value_or(QStringConverter::System));
  // out << line << "\n";
  // file.close();

  qDebug() << "MATLAB脚本已写入:" << scriptPath;
}


void net_mainwindow::onStopProcess()
{

    killOwnedProcesses();
    switchUI(0);
}
void net_mainwindow::killOwnedProcesses()
{
    auto killOne = [](QPointer<QProcess> p) {
        if (!p) return;
        if (p->state() != QProcess::NotRunning) {
            p->kill();
            p->waitForFinished(2000);
        }
    };
    killOne(m_p1_matlab);
    killOne(m_p2_pos_zx);
    killOne(m_p3_jihe);
    // killOne(m_p3);
}