#include "explorerwidget.h"

#include <QAbstractItemModel>
#include <QEvent>
#include <QFileIconProvider>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVector>

#include "../../database/projects.h"
#include "ui/windows/downloadwindow.h"
#include "ui/windows/importromdialog.h"
#include <lt/project/project.h>

#include <logger.h>
#include <memory>
#include <uiutil.h>

namespace detail
{
/* Traverses up the tree for a project. Returns
 * lt::ProjectPtr() if the search fails. */
lt::ProjectPtr find_project(QModelIndex index)
{
    while (index.isValid())
    {
        QVariant pData = index.data(Qt::UserRole);
        if (pData.canConvert<lt::ProjectPtr>())
        {
            return pData.value<lt::ProjectPtr>();
            break;
        }
        index = index.parent();
    }
    return lt::ProjectPtr();
}
} // namespace detail

ExplorerWidget::ExplorerWidget(QWidget * parent) : QWidget(parent)
{
    tree_ = new QTreeView;
    tree_->setHeaderHidden(true);
    tree_->installEventFilter(this);

    auto * layout = new QVBoxLayout;
    layout->addWidget(tree_);

    setLayout(layout);

    tree_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree_, &QTreeView::customContextMenuRequested, this,
            &ExplorerWidget::showContextMenu);

    connect(tree_, &QTreeView::activated, this, &ExplorerWidget::onActivated);
}

void ExplorerWidget::showContextMenu(const QPoint & point)
{
    QModelIndex index = tree_->indexAt(point);
    populateMenu(index);
    menu_.exec(tree_->viewport()->mapToGlobal(point));
}

void ExplorerWidget::setModel(QAbstractItemModel * model)
{
    tree_->setModel(model);
}

void ExplorerWidget::populateMenu(const QModelIndex & index)
{
    QVariant data = index.data(Qt::UserRole);
    menu_.setProject(detail::find_project(index));

    if (data.canConvert<lt::Rom::MetaData>())
        menu_.setRom(data.value<lt::Rom::MetaData>().path.filename().string());
    else if (data.canConvert<lt::Tune::MetaData>())
        menu_.setTune(
            data.value<lt::Tune::MetaData>().path.filename().string());
}

bool ExplorerWidget::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete)
        {
            // Ugly hack with the menu
            populateMenu(tree_->currentIndex());
            menu_.onDelete();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ExplorerWidget::onActivated(const QModelIndex & index)
{
    QVariant data = index.data(Qt::UserRole);
    if (data.canConvert<lt::Tune::MetaData>())
    {
        auto meta = data.value<lt::Tune::MetaData>();
        // Get project
        lt::ProjectPtr project = detail::find_project(index);
        if (!project)
            return;

        catchWarning(
            [&]() {
                lt::TunePtr tune =
                    project->loadTune(meta.path.filename().string());
                if (!tune)
                    return;
                emit tuneOpened(tune);
            },
            tr("Error loading tune"));
    }
}

ExplorerMenu::ExplorerMenu(QWidget * parent) : QMenu(parent)
{
    actionNewProject_ =
        new QAction(QIcon(":/icons/new_file.svg"), tr("New Project"), this);
    actionDownloadRom_ =
        new QAction(QIcon(":/icons/download.svg"), tr("Download ROM"), this);
    actionImportRom_ = new QAction(tr("Import ROM"), this);
    actionDelete_ = new QAction(QIcon(":/icons/trash.svg"), tr("Delete"), this);
    actionCreateTune_ =
        new QAction(QIcon(":/icons/file_text.svg"), tr("Create Tune"), this);
    actionDuplicate_ =
        new QAction(QIcon(":/icons/copy.svg"), tr("Duplicate"), this);

    addAction(actionNewProject_);
    addAction(actionDownloadRom_);
    addAction(actionImportRom_);
    addAction(actionDelete_);
    addAction(actionCreateTune_);
    addAction(actionDuplicate_);

    actionDownloadRom_->setEnabled(false);
    actionImportRom_->setEnabled(false);
    actionDelete_->setEnabled(false);
    actionCreateTune_->setEnabled(false);
    actionDuplicate_->setEnabled(false);

    connect(actionDownloadRom_, &QAction::triggered, [this]() {
        DownloadWindow dlg(project_);
        dlg.setModal(true);
        dlg.exec();
    });

    connect(actionImportRom_, &QAction::triggered, [this]() {
        ImportRomDialog dlg(project_);
        dlg.setModal(true);
        dlg.exec();
    });

    connect(actionDelete_, &QAction::triggered, this, &ExplorerMenu::onDelete);
}

void ExplorerMenu::onDelete()
{
    if (!project_)
        return;
    if (!tuneFilename_.empty())
    {
        if (QMessageBox::question(
                nullptr, tr("Delete Tune?"),
                tr("Are you sure you want to delete this tune?")) !=
            QMessageBox::Yes)
            return;
        project_->deleteTune(tuneFilename_);
        return;
    }
    if (!romFilename_.empty())
    {
        if (QMessageBox::question(
                nullptr, tr("Delete ROM?"),
                tr("Are you sure you want to delete this ROM? Tunes that "
                   "depend on it will need to be rebound to a new ROM.")) !=
            QMessageBox::Yes)
            return;
        project_->deleteRom(romFilename_);
        return;
    }
}

void ExplorerMenu::setProject(lt::ProjectPtr project)
{
    project_ = std::move(project);
    actionDownloadRom_->setEnabled(!!project_);
    actionImportRom_->setEnabled(!!project_);

    actionDelete_->setEnabled(false);
    actionCreateTune_->setEnabled(false);
    actionDuplicate_->setEnabled(false);
}

void ExplorerMenu::setRom(const std::string & filename)
{
    romFilename_ = filename;
    tuneFilename_.clear();
    actionDelete_->setEnabled(!romFilename_.empty());
    actionCreateTune_->setEnabled(!romFilename_.empty());
    actionDuplicate_->setEnabled(!romFilename_.empty());
}

void ExplorerMenu::setTune(const std::string & filename)
{
    tuneFilename_ = filename;
    romFilename_.clear();
    actionDelete_->setEnabled(!tuneFilename_.empty());
    actionCreateTune_->setEnabled(false);
    actionDuplicate_->setEnabled(!tuneFilename_.empty());
}