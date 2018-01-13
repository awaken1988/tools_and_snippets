/*
 * detailgui.cpp
 *
 *  Created on: Jan 13, 2018
 *      Author: martin
 */

#include "detaildui.h"
#include <QGridLayout>
#include <QLabel>
#include <QMimeDatabase>
#include <QFileIconProvider>



namespace detailgui
{
	constexpr int SIDES = 2;

	void impl_show_path(fsdiff::diff_t* aDiff,
						QGridLayout* aGrid,
						QWidget* aParent,
						fsdiff::diff_t::idx_t aIdx)
	{
		using namespace boost::filesystem;

		//symbol
		{
			QFileIconProvider icon_provider;
			QIcon icon = icon_provider.icon( QFileInfo(aDiff->fullpath[aIdx].string().c_str()) );

			QLabel* icon_label = new QLabel(aParent);
			icon_label->setPixmap(icon.pixmap(80,80));

			aGrid->addWidget(icon_label, aGrid->rowCount(), 1);

		}

		//last name
		{
			QString path = aDiff->getLastName(aIdx).string().c_str();
			QLabel* fullPathText 	= new QLabel("Name:", aParent);
			QLabel* fullPath 		= new QLabel(path, aParent);
			aGrid->addWidget(fullPathText, aGrid->rowCount(), 0);
			aGrid->addWidget(fullPath, aGrid->rowCount()-1, 1);
		}

		//full path
		{
			QString path = aDiff->fullpath[aIdx].string().c_str();
			QLabel* fullPathText 	= new QLabel("Full Path:", aParent);
			QLabel* fullPath 		= new QLabel(path, aParent);
			aGrid->addWidget(fullPathText, aGrid->rowCount(), 0);
			aGrid->addWidget(fullPath, aGrid->rowCount()-1, 1);
		}

		//base dir
		{
			QString path = aDiff->baseDir[aIdx].string().c_str();
			QLabel* fullPathText 	= new QLabel("Base Path:", aParent);
			QLabel* fullPath 		= new QLabel(path, aParent);
			aGrid->addWidget(fullPathText, aGrid->rowCount(), 0);
			aGrid->addWidget(fullPath, aGrid->rowCount()-1, 1);
		}

		//mime
		{
			QString type = "unkown file";

			if( is_directory( aDiff->fullpath[aIdx] ) ) {
				type = "dir";
			} else {
				QMimeDatabase db;
				QMimeType mime = db.mimeTypeForFile(aDiff->fullpath[aIdx].string().c_str());
				type = mime.name();
			}

			QLabel* fullPathText 	= new QLabel("Type:", aParent);
			QLabel* fullPath 		= new QLabel(type, aParent);
			aGrid->addWidget(fullPathText, aGrid->rowCount(), 0);
			aGrid->addWidget(fullPath, aGrid->rowCount()-1, 1);
		}

	}

	void show(fsdiff::diff_t* aDiff, QWidget* aLeft, QWidget* aRight)
	{
		using namespace fsdiff;

		for(int iSide=0; iSide<SIDES; iSide++) {
			auto curr = 0 == iSide  ? aLeft : aRight;

			QGridLayout* sideLayout = new QGridLayout();

			if( cause_t::ADDED == aDiff->cause && iSide != diff_t::RIGHT
				|| cause_t::DELETED == aDiff->cause && iSide != diff_t::LEFT )
			{
				QLabel* lbl = new QLabel( fsdiff::cause_t_str(aDiff->cause).c_str() , curr);
				sideLayout->addWidget(lbl, sideLayout->rowCount(), 0, 2, 1);
				sideLayout->setColumnStretch(0, 1);
				curr->setLayout(sideLayout);
			}
			else
			{
				impl_show_path(aDiff, sideLayout, curr, (diff_t::idx_t)iSide);

				sideLayout->setColumnStretch(1, 1);
				QWidget* wg = new QWidget(curr);
				sideLayout->addWidget(wg, sideLayout->rowCount(), 0);
				sideLayout->setRowStretch(sideLayout->rowCount()-1, 1);
			}

			int row = 0;

			curr->setLayout(sideLayout);

		}
	}
}

