#include "DbManagerWindow.h"
#include <QTabWidget>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDateEdit>
#include <QCheckBox>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>

static constexpr const char* kOrg = "DemoCo";
static constexpr const char* kApp = "QtDbManagerDemo";
static constexpr const char* kSettingsDbPathKey = "db/path";

DbManagerWindow::DbManagerWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle(u8"数据库管理");
    buildUi();
    wireSignals();
    loadSettingsAndAutoConnect();
    refreshView(); // 初次加载为空也没关系
}

DbManagerWindow::~DbManagerWindow() {
    closeDatabase();
}

void DbManagerWindow::buildUi() {
    tabs_ = new QTabWidget(this);

    // ===== 设置页 =====
    tabSettings_ = new QWidget(this);
    auto *fset = new QFormLayout(tabSettings_);
    editDbPath_ = new QLineEdit(tabSettings_);
    btnBrowseDb_ = new QPushButton(u8"选择…", tabSettings_);
    btnConnectDb_ = new QPushButton(u8"连接数据库", tabSettings_);
    lblConnState_ = new QLabel(u8"未连接", tabSettings_);

    auto *dbPathRow = new QWidget(tabSettings_);
    { auto *h = new QHBoxLayout(dbPathRow); h->setContentsMargins(0,0,0,0);
      h->addWidget(editDbPath_, 1); h->addWidget(btnBrowseDb_); }

    fset->addRow(u8"数据库文件：", dbPathRow);
    fset->addRow(u8"连接状态：", lblConnState_);
    fset->addRow(btnConnectDb_);
    tabSettings_->setLayout(fset);

    // ===== 入库页 =====
    tabIngest_ = new QWidget(this);
    auto *fins = new QFormLayout(tabIngest_);
    editDir_ = new QLineEdit(tabIngest_);
    btnBrowseDir_ = new QPushButton(u8"选择目录…", tabIngest_);
    comboType_ = new QComboBox(tabIngest_);
    comboType_->addItems({ "原始数据","预处理后产品","反演结果", "应用结果" });
    editNote_ = new QTextEdit(tabIngest_);
    editNote_->setPlaceholderText(u8"填写说明/备注...");
    btnInsert_ = new QPushButton(u8"入库", tabIngest_);
    lblInsertTip_ = new QLabel(tabIngest_);

    auto *dirRow = new QWidget(tabIngest_);
    { auto *h = new QHBoxLayout(dirRow); h->setContentsMargins(0,0,0,0);
      h->addWidget(editDir_, 1); h->addWidget(btnBrowseDir_); }

    fins->addRow(u8"数据目录：", dirRow);
    fins->addRow(u8"数据类型：", comboType_);
    fins->addRow(u8"说明：", editNote_);
    fins->addRow(btnInsert_);
    fins->addRow(lblInsertTip_);
    tabIngest_->setLayout(fins);

    // ===== 查看页 =====
    tabView_ = new QWidget(this);
    auto *vbox = new QVBoxLayout(tabView_);

    // 过滤区
    auto *filterRow = new QWidget(tabView_);
    auto *fh = new QHBoxLayout(filterRow); fh->setContentsMargins(0,0,0,0);
    comboFilterType_ = new QComboBox(filterRow);
    comboFilterType_->addItems({ u8"全部","原始数据","预处理后产品","反演结果", "应用结果" });
    cbEnableDate_ = new QCheckBox(u8"启用日期范围", filterRow);
    dateFrom_ = new QDateEdit(filterRow); dateFrom_->setCalendarPopup(true);
    dateTo_ = new QDateEdit(filterRow);   dateTo_->setCalendarPopup(true);
    dateFrom_->setDate(QDate::currentDate().addMonths(-1));
    dateTo_->setDate(QDate::currentDate());
    btnApplyFilter_ = new QPushButton(u8"确定", filterRow);
    btnClearFilter_ = new QPushButton(u8"清空", filterRow);

    fh->addWidget(new QLabel(u8"类型：", filterRow));
    fh->addWidget(comboFilterType_);
    fh->addSpacing(16);
    fh->addWidget(cbEnableDate_);
    fh->addWidget(new QLabel(u8"从：", filterRow));
    fh->addWidget(dateFrom_);
    fh->addWidget(new QLabel(u8"到：", filterRow));
    fh->addWidget(dateTo_);
    fh->addSpacing(16);
    fh->addWidget(btnApplyFilter_);
    fh->addWidget(btnClearFilter_);
    fh->addStretch();

    // 表格
    table_ = new QTableView(tabView_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_->horizontalHeader()->setStretchLastSection(true);

    // 统计
    lblCount_ = new QLabel(u8"共 0 条", tabView_);

    vbox->addWidget(filterRow);
    vbox->addWidget(table_, 1);
    vbox->addWidget(lblCount_);
    tabView_->setLayout(vbox);

    // Tab 容器
    tabs_->addTab(tabView_,   u8"查看");
    tabs_->addTab(tabIngest_, u8"入库");
    tabs_->addTab(tabSettings_, u8"设置");

    // 总体布局
    auto *root = new QVBoxLayout(this);
    root->addWidget(tabs_);
    setLayout(root);

    // 模型
    model_ = new QSqlQueryModel(this);
    table_->setModel(model_);
}

void DbManagerWindow::wireSignals() {
    connect(btnBrowseDb_, &QPushButton::clicked, this, [this] {
        const QString p = QFileDialog::getOpenFileName(this, u8"选择数据库文件",
                            QString(), u8"SQLite 数据库 (*.sqlite *.db);;所有文件 (*.*)");
        if (!p.isEmpty()) editDbPath_->setText(p);
    });
    connect(btnConnectDb_, &QPushButton::clicked, this, [this]{
        if (openDatabase(editDbPath_->text())) {
            saveDbPathToSettings(editDbPath_->text());
            QMessageBox::information(this, u8"成功", u8"数据库连接成功。");
            refreshView();
        }
    });

    connect(btnBrowseDir_, &QPushButton::clicked, this, [this]{
        const QString d = QFileDialog::getExistingDirectory(this, u8"选择数据目录");
        if (!d.isEmpty()) editDir_->setText(d);
    });
    connect(btnInsert_, &QPushButton::clicked, this, &DbManagerWindow::doInsert);

    connect(btnApplyFilter_, &QPushButton::clicked, this, &DbManagerWindow::refreshView);
    connect(btnClearFilter_, &QPushButton::clicked, this, [this]{
        comboFilterType_->setCurrentIndex(0);
        cbEnableDate_->setChecked(false);
        dateFrom_->setDate(QDate::currentDate().addMonths(-1));
        dateTo_->setDate(QDate::currentDate());
        refreshView();
    });

    connect(table_, &QTableView::doubleClicked, this, &DbManagerWindow::onTableDoubleClicked);
    connect(table_, &QTableView::customContextMenuRequested,
            this, &DbManagerWindow::onTableContextMenuRequested);
}

void DbManagerWindow::loadSettingsAndAutoConnect() {
    QSettings s(kOrg, kApp);
    const QString path = s.value(kSettingsDbPathKey).toString();
    if (!path.isEmpty()) {
        editDbPath_->setText(path);
        openDatabase(path); // 失败不弹窗，留到用户手动连接
    }
}

void DbManagerWindow::saveDbPathToSettings(const QString& p) {
    QSettings s(kOrg, kApp);
    s.setValue(kSettingsDbPathKey, p);
}

QString DbManagerWindow::connName() { return "app-entries"; }

bool DbManagerWindow::openDatabase(const QString& filePath) {
    closeDatabase();

    if (filePath.isEmpty()) {
        QMessageBox::warning(this, u8"提示", u8"请先选择数据库文件。");
        lblConnState_->setText(u8"未连接");
        return false;
    }

    if (QSqlDatabase::contains(connName())) {
        db_ = QSqlDatabase::database(connName());
    } else {
        db_ = QSqlDatabase::addDatabase("QSQLITE", connName());
    }
    db_.setDatabaseName(filePath);

    if (!db_.open()) {
        const QString err = db_.lastError().text();
        QMessageBox::critical(this, u8"连接失败", u8"无法打开数据库：\n" + err);
        lblConnState_->setText(u8"未连接");
        return false;
    }
    lblConnState_->setText(u8"已连接：" + filePath);
    return true;
}

void DbManagerWindow::closeDatabase() {
    if (db_.isValid()) {
        db_.close();
        db_ = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName()); // 断开连接
    }
}

bool DbManagerWindow::ensureOpen() {
    if (!db_.isValid() || !db_.isOpen()) {
        QMessageBox::warning(this, u8"未连接", u8"请先在“设置”页连接数据库。");
        return false;
    }
    return true;
}

void DbManagerWindow::doInsert() {
    if (!ensureOpen()) return;

    const QString dir = editDir_->text().trimmed();
    const QString type = comboType_->currentText().trimmed();
    const QString note = editNote_->toPlainText().trimmed();

    if (dir.isEmpty()) {
        lblInsertTip_->setText(u8"⚠ 请先选择数据目录。");
        return;
    }

    QSqlQuery q(db_);
    q.prepare("INSERT INTO entries(dir, created_at, type, note) VALUES(?, ?, ?, ?)");
    const QString nowIso = QDateTime::currentDateTime().toString(Qt::ISODate);
    q.addBindValue(dir);
    q.addBindValue(nowIso);
    q.addBindValue(type);
    q.addBindValue(note);

    if (!q.exec()) {
        lblInsertTip_->setText(u8"❌ 入库失败：" + q.lastError().text());
        return;
    }
    lblInsertTip_->setText(u8"✅ 入库成功。时间：" + nowIso);
    editNote_->clear();
    refreshView();
}

QString DbManagerWindow::isoStartOfDay(const QDate& d) {
    return QDateTime(d, QTime(0,0,0)).toString(Qt::ISODate);
}
QString DbManagerWindow::isoEndOfDay(const QDate& d) {
    return QDateTime(d, QTime(23,59,59,999)).toString(Qt::ISODateWithMs);
}

QString DbManagerWindow::buildFilterSqlWhere() const {
    QStringList parts;

    // 类型
    const QString t = comboFilterType_->currentText();
    if (t != u8"全部") {
        parts << QString("type = '%1'").arg(QString(t).replace('\'',"''"));
    }

    // 日期（created_at 为 TEXT，假定 ISO8601，可直接字符串比较）
    if (cbEnableDate_->isChecked()) {
        const QString s = isoStartOfDay(dateFrom_->date());
        const QString e = isoEndOfDay(dateTo_->date());
        parts << QString("created_at >= '%1' AND created_at <= '%2'").arg(s, e);
    }

    if (parts.isEmpty()) return QString();
    return "WHERE " + parts.join(" AND ");
}

void DbManagerWindow::refreshView() {
    if (!db_.isValid() || !db_.isOpen()) {
        model_->setQuery(QSqlQuery()); // 清空
        lblCount_->setText(u8"未连接数据库");
        return;
    }

    const QString where = buildFilterSqlWhere();
    const QString sql = QString(
        "SELECT id, dir, created_at, type, note "
        "FROM entries %1 "
        "ORDER BY datetime(created_at) DESC, id DESC").arg(where);

    model_->setQuery(sql, db_);

    if (model_->lastError().isValid()) {
        QMessageBox::critical(this, u8"查询失败", model_->lastError().text());
        return;
    }

    model_->setHeaderData(0, Qt::Horizontal, u8"id");
    model_->setHeaderData(1, Qt::Horizontal, u8"目录");
    model_->setHeaderData(2, Qt::Horizontal, u8"入库时间");
    model_->setHeaderData(3, Qt::Horizontal, u8"类型");
    model_->setHeaderData(4, Qt::Horizontal, u8"说明");

    table_->resizeColumnsToContents();
    lblCount_->setText(u8"共 " + QString::number(model_->rowCount()) + u8" 条");
}

void DbManagerWindow::onTableDoubleClicked(const QModelIndex& idx) {
    if (!idx.isValid()) return;
    const int row = idx.row();
    const QModelIndex dirIdx = model_->index(row, 1); // dir 列
    const QString dir = model_->data(dirIdx).toString();
    if (!dir.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    }
}

void DbManagerWindow::onTableContextMenuRequested(const QPoint& pos) {
    const QModelIndex idx = table_->indexAt(pos);
    QMenu menu(this);
    QAction* actDel = menu.addAction(u8"删除该条目");
    QAction* chosen = menu.exec(table_->viewport()->mapToGlobal(pos));
    if (chosen == actDel) {
        if (!idx.isValid()) return;
        const int row = idx.row();
        const qint64 id = model_->data(model_->index(row, 0)).toLongLong();
        if (QMessageBox::question(this, u8"确认删除",
                                  u8"确定删除 id=" + QString::number(id) + u8" 的记录吗？")
            == QMessageBox::Yes) {
            if (deleteRowById(id)) {
                refreshView();
            } else {
                QMessageBox::warning(this, u8"删除失败", u8"请查看日志或数据库状态。");
            }
        }
    }
}

bool DbManagerWindow::deleteRowById(qint64 id) {
    if (!ensureOpen()) return false;
    QSqlQuery q(db_);
    q.prepare("DELETE FROM entries WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("Delete failed: %s", q.lastError().text().toUtf8().constData());
        return false;
    }
    return true;
}
