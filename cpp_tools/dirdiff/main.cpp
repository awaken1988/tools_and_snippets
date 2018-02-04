#include "treemodel.h"
#include "sortfilterproxy.h"
#include "maingui.h"
#include <QApplication>
#include <QtGui>
#include <QWidget>
#include <QTreeView>
#include <QDesktopWidget>
#include <QMainWindow>

//TODO: QT redraw scrollbars
//TODO: select directory dialog
//TODO: detail box and to long strings

//FIXME: all filesystem actions should be moved to fsdiff
//			- saved in a try/catch call
//			- can return nothing

int main(int argc, char **argv)
{
	QApplication::setSetuidAllowed(true);
	QApplication app(argc, argv);

	path  left("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets/");
	path right("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets_copy/");

	//path  left("/media/xc3po_root/.xc3po_snapshots/2017_07_03__20_30/");
	//path right("/media/xc3po_root/.xc3po_snapshots/2018_01_26__22_37/");

	auto left_tree = fsdiff::list_dir_rekursive(left);
	auto difftree = fsdiff::compare(left, right);

	//fsdiff::dump( difftree );

	MainGui gui( difftree );
	gui.show();

	QMainWindow mainWindow;
	gui.resize(QDesktopWidget().availableGeometry(&mainWindow).size() * 0.7);;

//	TreeModel model(nullptr);
//	QTreeView view(nullptr);
//	SortFilterProxy sfp;
//
//	sfp.setSourceModel(&model);
//
//	view.setModel(&sfp);
//	view.setMinimumWidth(800);
//	view.setMinimumHeight(800);
//	view.show();
//
//	//sfp.setFilterRole( Qt::DisplayRole );
//	//sfp.setFilterKeyColumn(0);

	return app.exec();

	//filesys::diff_tree(path("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets/"), path("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets_copy/"));
}
