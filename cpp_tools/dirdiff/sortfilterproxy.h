/*
 * sortfilterproxy.h
 *
 *  Created on: Dec 31, 2017
 *      Author: martin
 */

#ifndef SORTFILTERPROXY_H_
#define SORTFILTERPROXY_H_

#include <QSortFilterProxyModel>
#include "filesys.h"

class SortFilterProxy : public QSortFilterProxyModel {
public:
	SortFilterProxy(QObject* aParent);
	virtual ~SortFilterProxy();

	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif /* SORTFILTERPROXY_H_ */
