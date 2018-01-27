/*
 * maingui.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: martin
 */

#include "maingui.h"
#include "detaildui.h"
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
	}

	//left right box
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
	{
		for(size_t iSide=0; iSide<m_cmp_detail.size(); iSide++) {
			delete m_cmp_detail[iSide];
		}
		init_left_right_info();
	}

	detailgui::show(diff, m_cmp_detail[0], m_cmp_detail[1]);
}

void MainGui::init_left_right_info()
{
	{
		m_cmp_detail = { new QGroupBox("Left"), new QGroupBox("Right") };

		for(int i=0; i<2; i++) {

			m_cmp_detail[i]->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );


			m_layout->addWidget(m_cmp_detail[i], 1, i);
		}
	}
}






