/*
 * maingui.h
 *
 *  Created on: Jan 3, 2018
 *      Author: martin
 */

#ifndef MAINGUI_H_
#define MAINGUI_H_

#include <array>
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>
#include "fsdiff.h"
#include "treemodel.h"
#include "sortfilterproxy.h"



class MainGui : public QWidget
{
public:
	MainGui( std::shared_ptr<fsdiff::diff_t> aDiffTree );
	virtual ~MainGui();

public slots:
	void clicked_diffitem(const QModelIndex &index);

protected:
	void init_left_right_info();

protected:
	std::array<QGroupBox*, 2> m_cmp_detail;
	TreeModel* m_model;
	SortFilterProxy* m_filter;
	QGridLayout* m_layout;
};

#endif /* MAINGUI_H_ */
