#include "dialog_backdoor.h"

#include <QFileDialog>
#include <QFileInfo>

#include "ConfigManager.h"
#include "ui_dialog_backdoor.h"

Dialog_BackDoor::Dialog_BackDoor(QWidget* parent)
    : QDialog(parent), ui(new Ui::Dialog_BackDoor) {
  ui->setupUi(this);

  setConnection();
  setUI();
}

Dialog_BackDoor::~Dialog_BackDoor() { delete ui; }

void Dialog_BackDoor::setUI() {
  // 读取配置，设置一下脚本根目录
  if (ConfigManager::instance().getValue("exe_Path/exe3_2Path").isValid()) {
    ui->txt_3_2->setText(ConfigManager::instance()
                             .getValue("exe_Path/exe3_2Path")
                             .toString());  // 脚本注入目录

    path3_2 = ui->txt_3_2->text();
  }
}

void Dialog_BackDoor::setConnection() {
  connect(ui->btn_select_3_2, &QPushButton::clicked, this, [this]() {
    // 打开选择窗口
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), "C:/",
                                                    tr("所有文件 (*.*)"));
    if (!filePath.isEmpty()) {
      ui->txt_3_2->setText(filePath);
      // 写入本地配置
      ConfigManager::instance().setValue("exe_Path/exe3_2Path",
                                         filePath);  // 保存设置
      // 写道成员变量
      path3_2 = filePath;
    }
  });

  connect(ui->btn_OK, &QPushButton::clicked, this, [this]() { this->hide(); });
}