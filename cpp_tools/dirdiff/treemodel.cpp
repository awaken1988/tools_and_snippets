
#include "treemodel.h"
#include <QStringList>
#include <QBrush>

enum class column_e {
    ITEM_NAME=0,
    ITEM_CAUSE,
    DIFF_SIZE,
    LEN,
};

TreeModel::TreeModel(QObject *parent, std::shared_ptr<fsdiff::diff_t> aDiffTree)
    : QAbstractItemModel(parent)
{
    rootItem = aDiffTree;
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



    if (role == Qt::DisplayRole) {
        if( nullptr == index.internalPointer() )
        	return QVariant();

        fsdiff::diff_t* item = static_cast<fsdiff::diff_t*>(index.internalPointer());

        if( index.column() == static_cast<int>(column_e::ITEM_NAME) ) {
            return QString( item->getLastName().c_str());
        }
        else if(  index.column() == static_cast<int>(column_e::ITEM_CAUSE)  ) {
			return fsdiff::cause_t_str( item->cause ).c_str();
		 }
        else if(  index.column() == static_cast<int>(column_e::DIFF_SIZE)  ) {
            return "blubb";
        }
        else {
            return QVariant();
        }
    }
    else if( role == Qt::BackgroundRole ) {
    	using namespace fsdiff;

    	fsdiff::diff_t* item = static_cast<fsdiff::diff_t*>(index.internalPointer());

    	switch( item->cause )
    	{
    	case cause_t::ADDED: 		return QBrush( Qt::green );
    	case cause_t::DELETED:		return QBrush( Qt::cyan );
    	case cause_t::DIR_TO_FILE:	return QBrush( Qt::red );
    	case cause_t::FILE_TO_DIR:	return QBrush( Qt::red );
    	//case cause_t::CONTENT:		return QBrush( Qt::red );
    	default:					return QVariant();
    	}
    }


    return QVariant();
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

    fsdiff::diff_t* parentItem;

    if (!parent.isValid()) {
        parentItem = rootItem.get();
    }
    else {
        parentItem = static_cast<fsdiff::diff_t*>(parent.internalPointer());
    }

    if( row >= parentItem->childs.size() ) {
    	return QModelIndex();
    }

    return createIndex(row, column, parentItem->childs[row].get());
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    fsdiff::diff_t* childItem = static_cast<fsdiff::diff_t*>(index.internalPointer());
    if( nullptr == childItem )
    	return QModelIndex();
    fsdiff::diff_t* parentItem = childItem->parent.get();

    if (parentItem == rootItem.get() || nullptr == parentItem) {
        //cout<<"failed "<<parentItem<<endl;
    	return QModelIndex();
    }

    int row = 0;
    for(int i=0; i<parentItem->childs.size(); i++) {
        if( parentItem->childs[i].get() == childItem  ) {
            row = i;
        }
    }

    return createIndex(row, 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    fsdiff::diff_t* parentItem = nullptr;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem.get();
    else
        parentItem = static_cast<fsdiff::diff_t*>(parent.internalPointer());

    if( nullptr == parentItem )
    	return 0;

    int ret = parentItem->childs.size();
    //cout<<ret<<endl;
    return ret;
}

void TreeModel::setupModelData()
{
    path  left("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets/");
    path right("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets_copy/");

    rootItem = fsdiff::compare(left, right);
    //rootItem = fsdiff::list_dir_rekursive(left);
}
