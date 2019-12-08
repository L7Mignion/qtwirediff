#include "mainwindow.h"
#include "difftreemodel.h"
#include <QDebug>


static void dumpDiffItem(DiffItem *n, int dep=0)
{
    qDebug() << QString(dep, '>') << " " << dep << "=" << n->node->p->depth() << "|" << n->node->p->val;
    for (int i = 0; i < n->children.size(); i++) {
        dumpDiffItem(n->children[i], dep+1);
    }
}


DiffTreeModel::DiffTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

DiffTreeModel::~DiffTreeModel()
{

}

void DiffTreeModel::updateDiff()
{
    QVector<DiffNode>* diff = MainWindow::instance->getDiff();
    QVector<DiffItem*> parents;

    root = new DiffItem();
    root->parent = nullptr;

    if (!diff || diff->isEmpty()) {
        return;
    }


    root->node = &(*diff)[0];

    // depth 0 => root => no parents
    parents.append(nullptr);
    // depth 1 => parent is root
    parents.append(root);

    // first pass, size parents array right
    for (int i = 1; i < diff->size(); i++) {
        DiffNode *n = &(*diff)[i];
        int depth = n->p->depth();
        if (parents.size() < depth+2)
            parents.resize(depth+2);
    }

    for (int j = 1; j < parents.size(); j++)
        parents[j] = root;


    for (int i = 1; i < diff->size(); i++) {
        DiffItem *it = new DiffItem();
        DiffNode *n = &(*diff)[i];
        it->node = n;

        int depth = n->p->depth();
        for (int j = depth+1; j < parents.size(); j++)
            parents[j] = it;
        it->parent = parents[depth];
        it->parent->children.append(it);
    }
    //dumpDiffItem(root);

}



int DiffTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant DiffTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    DiffItem *item = static_cast<DiffItem*>(index.internalPointer());
    return item->node->p->val;
}

Qt::ItemFlags DiffTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant DiffTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return "Text";
        default:
            return "foo";
        }
    }
    return QVariant();
}

QModelIndex DiffTreeModel::index(int row, int column, const QModelIndex &parent) const
{

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DiffItem *parentItem;

    if (!parent.isValid())
        parentItem = root;
    else
        parentItem = static_cast<DiffItem*>(parent.internalPointer());

    DiffItem *childItem = parentItem->children[row];
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex DiffTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    DiffItem *childItem = static_cast<DiffItem*>(index.internalPointer());
    DiffItem *parentItem = childItem->parent;

    if (parentItem == root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int DiffTreeModel::rowCount(const QModelIndex &parent) const
{
    DiffItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = root;
    else
        parentItem = static_cast<DiffItem*>(parent.internalPointer());

    return parentItem->children.size();
}