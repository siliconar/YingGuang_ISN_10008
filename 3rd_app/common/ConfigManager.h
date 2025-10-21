#ifndef ConfigManager_H
#define ConfigManager_H

#include <QMap>
#include <QSettings>
#include <QString>
#include <QVariant>

// 数据判读和统计
class ConfigManager {
 public:
  // 单例模式，确保全局唯一实例
  static ConfigManager& instance();

  // 删除拷贝构造和赋值操作
  ConfigManager(const ConfigManager&) = delete;
  ConfigManager& operator=(const ConfigManager&) = delete;

  // 设置配置项
  void setValue(const QString& key, const QVariant& value);

  // 读取配置项，带默认值
  QVariant getValue(const QString& key,
                    const QVariant& defaultValue = QVariant()) const;

  // 显式从磁盘加载所有配置
  QMap<QString, QVariant> loadFromDisk();

  // 清除所有配置
  void clear();

  // 同步配置到磁盘
  void sync();

 private:
  ConfigManager();      // 私有构造函数
  QSettings* settings;  // QSettings 对象
};

#endif  // ConfigManager_H
