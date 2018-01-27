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
#include <QPlainTextEdit>
#include <QPixmap>
#include <boost/filesystem/fstream.hpp>



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
			fullPath->setWordWrap(true);
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
		QString mime_type = "";
		{
			if( is_regular_file( aDiff->fullpath[aIdx] ) ) {
				QMimeDatabase db;
				QMimeType mime = db.mimeTypeForFile(aDiff->fullpath[aIdx].string().c_str());
				mime_type = mime.name();
			}

			QLabel* fullPathText 	= new QLabel("Type:", aParent);
			QLabel* fullPath 		= new QLabel(mime_type, aParent);
			aGrid->addWidget(fullPathText, aGrid->rowCount(), 0);
			aGrid->addWidget(fullPath, aGrid->rowCount()-1, 1);
		}

		//open diff item
		if( mime_type.startsWith("text") )
		{
			QPlainTextEdit* plainText = new QPlainTextEdit(aParent);

			QSizePolicy policy = plainText->sizePolicy();
			policy.setVerticalStretch(2);
			plainText->setSizePolicy(policy);

			boost::filesystem::ifstream file(aDiff->fullpath[aIdx]);
			string content;
			while( getline(file, content) ) {
				plainText->appendPlainText(content.c_str());
			}


			aGrid->addWidget(plainText, aGrid->rowCount(), 0, 3, 2);
		}
		else if( mime_type.startsWith("image") )
		{
			QLabel* lbl = new QLabel;

			QPixmap pxmp( aDiff->fullpath[aIdx].c_str() );
			pxmp =pxmp.scaledToHeight(80, Qt::FastTransformation );

			lbl->setPixmap(pxmp);



			aGrid->addWidget(lbl, aGrid->rowCount(), 0, 3, 2, Qt::AlignHCenter);
		}


	}

	void show(fsdiff::diff_t* aDiff, QWidget* aLeft, QWidget* aRight)
	{
		using namespace fsdiff;

		for(int iSide=0; iSide<SIDES; iSide++) {
			auto curr = 0 == iSide  ? aLeft : aRight;

			QGridLayout* sideLayout = new QGridLayout();

			if( (cause_t::ADDED == aDiff->cause && iSide != diff_t::RIGHT)
				|| (cause_t::DELETED == aDiff->cause && iSide != diff_t::LEFT) )
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

			curr->setLayout(sideLayout);

		}
	}
}

