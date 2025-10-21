#pragma once
#include <QLockFile>
#include <QStandardPaths>
#include <QDir>
#include <QString>

class SingleInstance_SharedMemory { // 名称沿用，内部已改为 QLockFile
public:
    explicit SingleInstance_SharedMemory(const QString& key,
                                         int staleMs = 5000,   // 陈旧锁判定时间
                                         int tryMs   = 100)    // 获取锁等待时长
        : key_(key),
          lockFile_(makeLockPath(key)),
          tryTimeoutMs_(tryMs)
    {
        lockFile_.setStaleLockTime(staleMs);
    }

    // 成为唯一实例返回 true；已有实例在运行返回 false
    bool ensureSingle() {
        return lockFile_.tryLock(tryTimeoutMs_);
    }

    // 调试/排查用：查看锁文件路径
    QString lockPath() const { return lockFile_.fileName(); }

private:
    static QString makeLockPath(const QString& key) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if (dir.isEmpty()) dir = QDir::tempPath();
        return QDir(dir).filePath(key + ".lock");
    }

private:
    QString   key_;
    QLockFile lockFile_;
    int       tryTimeoutMs_;
};
