#include "ConfigManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>


ConfigManager::ConfigManager() {
  // 设置配置文件路径，通常在用户目录下的 config 文件夹
  QString configPath =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  QDir dir(configPath);
  if (!dir.exists()) {
    dir.mkpath(".");  // 创建目录
  }

  // 使用 INI 格式存储，文件名为 appname.ini
  settings = new QSettings(configPath + "/app_yingguang.ini", QSettings::IniFormat);
}

ConfigManager& ConfigManager::instance() {
  static ConfigManager config;  // 静态实例，单例模式
  return config;
}

void ConfigManager::setValue(const QString& key, const QVariant& value) {
  settings->setValue(key, value);
}

QVariant ConfigManager::getValue(const QString& key,
                                 const QVariant& defaultValue) const {
  return settings->value(key, defaultValue);
}

QMap<QString, QVariant> ConfigManager::loadFromDisk() {
  QMap<QString, QVariant> configData;
  settings->sync();  // 确保从磁盘读取最新数据
  foreach (const QString& group, settings->childGroups()) {
    settings->beginGroup(group);
    foreach (const QString& key, settings->childKeys()) {
      QString fullKey = group + "/" + key;  // 构造完整键，如 "User/Name"
      configData[fullKey] = settings->value(key);
    }
    settings->endGroup();
  }
  return configData;
}

void ConfigManager::clear() { settings->clear(); }

void ConfigManager::sync() {
  settings->sync();  // 强制将配置写入磁盘
}