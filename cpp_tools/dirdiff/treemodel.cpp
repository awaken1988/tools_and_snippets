
#include "treemodel.h"
#include <QStringList>

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Files" << "Summary";
    setupModelData();
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<filesys::diff_t*>(parent.internalPointer())->childs.size();
    else
        return 1;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    filesys::diff_t* item = static_cast<filesys::diff_t*>(index.internalPointer());

    return QString( item->last_element().string().c_str() );
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    //if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    //    return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    filesys::diff_t* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<filesys::diff_t*>(parent.internalPointer());

    filesys::diff_t* childItem = parentItem->childs[row].get();
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    filesys::diff_t* childItem = static_cast<filesys::diff_t*>(index.internalPointer());
    filesys::diff_t* parentItem = childItem->parent.get();

    if (parentItem == rootItem || nullptr == parentItem) 
        return QModelIndex();

    int row = 0;
    for(int i=0; i<parentItem->childs.size(); i++) {
        if( parentItem->childs[i]->item == childItem->item ) {
            row = i;
        }
    }

    return createIndex(row, 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    filesys::diff_t* parentItem = nullptr;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<filesys::diff_t*>(parent.internalPointer());

    return parentItem->childs.size();
}

void TreeModel::setupModelData()
{
    rootItem = new filesys::diff_t(path("./"), filesys::cause_t::SAME, nullptr  ) ;
    
    path  left("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets/");
    path right("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets_copy/");

    rootItem->childs = filesys::iterate_dir_recursively(left, right, path(), nullptr);
    filesys::print_dir_recursive(rootItem->childs);
}