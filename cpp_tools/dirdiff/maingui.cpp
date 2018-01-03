/*
 * maingui.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: martin
 */

#include "maingui.h"


MainGui::MainGui( std::shared_ptr<fsdiff::diff_t> aDiffTree )
{
	m_filter = new SortFilterProxy();		//TODO: who is destroying this object
	m_model = new TreeModel(this, aDiffTree);

	m_filter->setSourceModel(m_model);


	QGridLayout* layout = new QGridLayout();
	m_layout = layout;

	//tree
	{
		QTreeView* treeView = new QTreeView(this);
		treeView->setModel(m_filter);
		layout->addWidget(treeView, 0, 0, 2, 0);

		QObject::connect(treeView, &QTreeView::clicked, this, &MainGui::clicked_diffitem);
	}

	//left right box
	init_left_right_info();

	{
		QPushButton* btn = new QPushButton("test", this);

		QObject::connect(btn, &QPushButton::clicked, [=](bool checked) {
			for(int iSide=0; iSide<m_cmp_detail.size(); iSide++) {
				qDeleteAll(m_cmp_detail[iSide]->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
			}
		});
	}

	setLayout( layout );
}

MainGui::~MainGui()
{

}

void MainGui::clicked_diffitem(const QModelIndex &index)
{
	using namespace fsdiff;

	QModelIndex sourceIndex = m_filter->mapToSource(index);
	if( !sourceIndex.isValid() )
		return;
	diff_t* diff = static_cast<diff_t*>(sourceIndex.internalPointer());
	if( nullptr == diff )
		return;

	cout<<"clicked"<<endl;

	//recreate widgets
	{
		for(int iSide=0; iSide<m_cmp_detail.size(); iSide++) {
			delete m_cmp_detail[iSide];
		}
		init_left_right_info();
	}

	for(int iSide=0; iSide<m_cmp_detail.size(); iSide++) {
		auto curr = m_cmp_detail[iSide];
		QGridLayout* sideLayout = new QGridLayout();

		//full path
		{
			QString path = diff->fullpath[iSide].string().c_str();
			QLabel* fullPathText 	= new QLabel("Full Path:", curr);
			QLabel* fullPath 		= new QLabel(path, curr);
			sideLayout->addWidget(fullPathText, 0, 0);
			sideLayout->addWidget(fullPath, 0, 1);
		}

		curr->setLayout(sideLayout);
	}

}

void MainGui::init_left_right_info()
{
	{
		m_cmp_detail = { new QGroupBox("Left"), new QGroupBox("Right") };

		for(int i=0; i<2; i++) {
			m_layout->addWidget(m_cmp_detail[i], 1, i);
		}
	}
}






