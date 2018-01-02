/*
 * sortfilterproxy.cpp
 *
 *  Created on: Dec 31, 2017
 *      Author: martin
 */

#include "sortfilterproxy.h"
#include "treemodel.h"

SortFilterProxy::SortFilterProxy() {
	// TODO Auto-generated constructor stub

}

SortFilterProxy::~SortFilterProxy() {
	// TODO Auto-generated destructor stub
}

bool SortFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	using namespace filesys;

	return true;
}

