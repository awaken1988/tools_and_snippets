/*
 * maingui.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: martin
 */

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

	m_filter->clear();
	m_filter->setSourceModel(m_model);

	QGridLayout* layout = new QGridLayout;
	m_layout = layout;

	//tree
	{
		QTreeView* treeView = new QTreeView(this);
		treeView->setModel(m_filter);
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
	return;

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

		if( cause_t::ADDED == diff->cause && iSide != diff_t::RIGHT
			|| cause_t::DELETED == diff->cause && iSide != diff_t::LEFT )
		{
			QWidget* wg = new QWidget(curr);
			sideLayout->addWidget(wg, sideLayout->rowCount(), 0);
		}

		int row = 0;

		//symbol
		{
			QFileIconProvider icon_provider;
			QIcon icon = icon_provider.icon( QFileInfo(diff->fullpath[iSide].string().c_str()) );

			QLabel* icon_label = new QLabel(curr);
			icon_label->setPixmap(icon.pixmap(80,80));

			sideLayout->addWidget(icon_label, row, 1);

		} row++;

		//full path
		{
			QString path = diff->fullpath[iSide].string().c_str();
			QLabel* fullPathText 	= new QLabel("Full Path:", curr);
			QLabel* fullPath 		= new QLabel(path, curr);
			sideLayout->addWidget(fullPathText, row, 0);
			sideLayout->addWidget(fullPath, row, 1);
		} row++;

		//base dir
		{
			QString path = diff->baseDir[iSide].string().c_str();
			QLabel* fullPathText 	= new QLabel("Base Dir:", curr);
			QLabel* fullPath 		= new QLabel(path, curr);
			sideLayout->addWidget(fullPathText, row, 0);
			sideLayout->addWidget(fullPath, row, 1);
		} row++;

		//last name
		{
			QString path = diff->getLastName().string().c_str();
			QLabel* fullPathText 	= new QLabel("Last Name:", curr);
			QLabel* fullPath 		= new QLabel(path, curr);
			sideLayout->addWidget(fullPathText, row, 0);
			sideLayout->addWidget(fullPath, row, 1);
		} row++;

		//last name
		{
			QString type = "unkown file";

			if( is_directory( diff->fullpath[iSide] ) ) {
				type = "dir";
			} else {
				QMimeDatabase db;
				QMimeType mime = db.mimeTypeForFile(diff->fullpath[iSide].string().c_str());
				type = mime.name();
			}

			QLabel* fullPathText 	= new QLabel("Type:", curr);
			QLabel* fullPath 		= new QLabel(type, curr);
			sideLayout->addWidget(fullPathText, row, 0);
			sideLayout->addWidget(fullPath, row, 1);
		} row++;

		//stretch
		{
			sideLayout->setColumnStretch(1, 1);

			QWidget* wg = new QWidget(curr);
			sideLayout->addWidget(wg, sideLayout->rowCount(), 0);
			sideLayout->setRowStretch(sideLayout->rowCount()-1, 1);
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






