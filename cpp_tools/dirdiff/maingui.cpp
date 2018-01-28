/*
 * maingui.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: martin
 */

#include <detailgui.h>
#include "maingui.h"
#include <QMimeDatabase>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QIcon>
#include <QPushButton>
#include <QLabel>

MainGui::MainGui( std::shared_ptr<fsdiff::diff_t> aDiffTree )
{
	m_filter = new SortFilterProxy(this);		//TODO: who is destroying this object
	m_model = new TreeModel(this, aDiffTree);

	if( m_with_filter ) {
		m_filter->clear();
		m_filter->setSourceModel(m_model);
	}

	QGridLayout* layout = new QGridLayout;
	m_layout = layout;

	//tree
	{
		QTreeView* treeView = new QTreeView(this);
		treeView->setModel( m_with_filter ? static_cast<QAbstractItemModel*>(m_filter) : static_cast<QAbstractItemModel*>(m_model));
		layout->addWidget(treeView, 0, 0, 1, 2);

		QObject::connect(treeView, &QTreeView::clicked, this, &MainGui::clicked_diffitem);

		treeView->setColumnWidth(0, 300);
	}

	//left right box
	m_detail_tab = new QTabWidget(this);
	layout->addWidget(m_detail_tab, 1, 0, 1, 2);
	init_left_right_info();

//	{
//		QPushButton* btn = new QPushButton("test", this);
//
//		QObject::connect(btn, &QPushButton::clicked, [=](bool checked) {
//			for(int iSide=0; iSide<m_cmp_detail.size(); iSide++) {
//				qDeleteAll(m_cmp_detail[iSide]->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
//			}
//		});
//	}

	setLayout( layout );
}

MainGui::~MainGui()
{

}

void MainGui::clicked_diffitem(const QModelIndex &index)
{
	using namespace fsdiff;

	QModelIndex sourceIndex = index;
	if( m_with_filter ) {
		sourceIndex = m_filter->mapToSource(index);
	}

	if( !sourceIndex.isValid() )
		return;
	diff_t* diff = static_cast<diff_t*>(sourceIndex.internalPointer());
	if( nullptr == diff )
		return;

	cout<<"clicked"<<endl;

	//recreate widgets
	init_left_right_info();


	auto tab_changed_fun = [&](int aIdx) {
		if( aIdx < 0)
			return;
		m_detail_tab_idx = aIdx;
	};

	//detail tab: filename, path, mime
	m_detail_tab->addTab(detailgui::show_detail(diff), "Details");
	QObject::connect(m_detail_tab, &QTabWidget::tabBarClicked, tab_changed_fun);

	//content
	const int content_tab_idx = m_detail_tab->addTab(detailgui::show_content(diff), "Content");
	QObject::connect(m_detail_tab, &QTabWidget::tabBarClicked, tab_changed_fun);

	//restore las tab
	m_detail_tab_idx = m_detail_tab->count() <= m_detail_tab_idx ? 0 : m_detail_tab_idx;
	m_detail_tab->setCurrentIndex(m_detail_tab_idx);


}

void MainGui::init_left_right_info()
{
		m_detail_tab->clear();
}






