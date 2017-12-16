#include "filesys.h"
#include "treemodel.h"
#include <QApplication>
#include <QtGui>
#include <QWidget>
#include <QTreeView>



int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    
    TreeModel model(nullptr);

    QTreeView view(nullptr);
    view.setModel(&model);
    view.setMinimumWidth(800);
    view.setMinimumHeight(800);
    view.show();


    return app.exec();
}
