#pragma once
#include <QDialog>
#include <QSqlDatabase>
#include <QDateTime>

class QTabWidget;
class QLineEdit;
class QPushButton;
class QComboBox;
class QTextEdit;
class QTableView;
class QSqlQueryModel;
class QDateEdit;
class QCheckBox;
class QLabel;

class DbManagerWindow : public QDialog {
    Q_OBJECT
public:
    explicit DbManagerWindow(QWidget* parent=nullptr);
    ~DbManagerWindow();

private:
    // UI
    QTabWidget* tabs_{};

    // --- 设置页 ---
    QWidget* tabSettings_{};
    QLineEdit* editDbPath_{};
    QPushButton* btnBrowseDb_{};
    QPushButton* btnConnectDb_{};
    QLabel* lblConnState_{};

    // --- 入库页 ---
    QWidget* tabIngest_{};
    QLineEdit* editDir_{};
    QPushButton* btnBrowseDir_{};
    QComboBox* comboType_{};
    QTextEdit* editNote_{};
    QPushButton* btnInsert_{};
    QLabel* lblInsertTip_{};

    // --- 查看页 ---
    QWidget* tabView_{};
    QComboBox* comboFilterType_{};
    QCheckBox* cbEnableDate_{};
    QDateEdit* dateFrom_{};
    QDateEdit* dateTo_{};
    QPushButton* btnApplyFilter_{};
    QPushButton* btnClearFilter_{};
    QTableView* table_{};
    QSqlQueryModel* model_{};
    QLabel* lblCount_{};

    // DB
    QSqlDatabase db_;
    bool ensureOpen();
    bool openDatabase(const QString& filePath);
    void closeDatabase();
    void loadSettingsAndAutoConnect();
    void saveDbPathToSettings(const QString&);

    // Actions
    void buildUi();
    void wireSignals();

    void doInsert();
    void refreshView();
    QString buildFilterSqlWhere() const;

    void onTableDoubleClicked(const QModelIndex& idx);
    void onTableContextMenuRequested(const QPoint& pos);
    bool deleteRowById(qint64 id);

    // helpers
    static QString connName();
    static QString isoStartOfDay(const QDate& d);
    static QString isoEndOfDay(const QDate& d);
};
