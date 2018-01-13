/*
 * sortfilterproxy.cpp
 *
 *  Created on: Dec 31, 2017
 *      Author: martin
 */

#include "sortfilterproxy.h"
#include "treemodel.h"
#include <iostream>

SortFilterProxy::SortFilterProxy(QObject* aParent)
	: QSortFilterProxyModel(aParent)
{
	// TODO Auto-generated constructor stub

}

SortFilterProxy::~SortFilterProxy() {
	// TODO Auto-generated destructor stub
}

bool SortFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	using namespace filesys;
	using namespace std;

	cout<<"sourceRow="<<sourceRow<<"; row="<<sourceParent.row()<<"; col="<<sourceParent.column()
			<<"; ptr="<<sourceParent.internalPointer()<<endl;

	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

	diff_t* left_ptr = static_cast<diff_t*>(index0.internalPointer());



	if( cause_t::ADDED == left_ptr->cause)
		return false;
	if( cause_t::REMOVED == left_ptr->cause)
		return false;

	return true;
}

bool SortFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	using namespace fsdiff;

	diff_t* left_ptr = static_cast<diff_t*>(left.internalPointer());
	diff_t* right_ptr = static_cast<diff_t*>(right.internalPointer());

	return left_ptr->debug_id < right_ptr->debug_id;

}
