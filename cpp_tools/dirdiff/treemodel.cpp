
#include "treemodel.h"
#include <QStringList>

enum class column_e {
    ITEM_NAME=0,
    DIFF_SIZE,
    LEN,
};

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Files" << "Summary";
    setupModelData();
}

TreeModel::~TreeModel()
{

}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    return static_cast<int>(column_e::LEN);
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if( nullptr == index.internalPointer() )
    	return QVariant();

    filesys::diff_t* item = static_cast<filesys::diff_t*>(index.internalPointer());

    if( index.column() == static_cast<int>(column_e::ITEM_NAME) ) {
        return QString( item->item.string().c_str() );
    } 
    else if(  index.column() == static_cast<int>(column_e::DIFF_SIZE)  ) {
        return "blubb";
    }
    else {
        return QVariant();
    }   
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
        parentItem = rootItem.get();
    else
        parentItem = static_cast<filesys::diff_t*>(parent.internalPointer());


    return createIndex(row, column, parentItem->childs_vec[row].get());
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    filesys::diff_t* childItem = static_cast<filesys::diff_t*>(index.internalPointer());
    if( nullptr == childItem )
    	return QModelIndex();
    filesys::diff_t* parentItem = childItem->parent.get();

    if (parentItem == rootItem.get() || nullptr == parentItem) {
        //cout<<"failed "<<parentItem<<endl;
    	return QModelIndex();
    }

    int row = 0;
    for(int i=0; i<parentItem->childs_vec.size(); i++) {
        if( parentItem->childs_vec[i]->item == childItem->item ) {
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
        parentItem = rootItem.get();
    else
        parentItem = static_cast<filesys::diff_t*>(parent.internalPointer());

    if( nullptr == parentItem )
    	return 0;

    int ret = parentItem->childs_vec.size();
    //cout<<ret<<endl;
    return ret;
}

void TreeModel::setupModelData()
{
    path  left("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets/");
    path right("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets_copy/");

    rootItem = filesys::diff_tree(left, right);

    filesys::print_dir_recursive(rootItem, 0);
}
