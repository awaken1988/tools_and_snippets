#include "treemodel.h"
#include "sortfilterproxy.h"
#include "maingui.h"
#include <vector>
#include <boost/filesystem.hpp>
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

auto getPathByArg(int argc, char **argv)
{
	std::vector<boost::filesystem::path> ret;

	for(int iArg=0; iArg<argc && ret.size() < 2; iArg++) {
		auto curr_path = boost::filesystem::path(argv[iArg]);

		if( boost::filesystem::is_directory(curr_path) ) {
			ret.push_back(curr_path);
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	QApplication::setSetuidAllowed(true);
	QApplication app(argc, argv);

	auto diff_paths = getPathByArg(argc, argv);

	if( diff_paths.size() < 2 ) {
		std::cout<<"no paths given as argument; TODO: show gui"<<endl;
		return 1;
	}

	auto difftree = fsdiff::compare(diff_paths[0], diff_paths[1]);

	MainGui gui( difftree );
	gui.show();

	QMainWindow mainWindow;
	gui.resize(QDesktopWidget().availableGeometry(&mainWindow).size() * 0.7);;


	return app.exec();
}

//path  left("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets/");
//path right("/home/martin/Dropbox/Programming/tools_and_snippets/cpp_snippets_copy/");
//path  left("/media/xc3po_root/.xc3po_snapshots/2017_07_03__20_30/");
//path right("/media/xc3po_root/.xc3po_snapshots/2018_01_26__22_37/");
