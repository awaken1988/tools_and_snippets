#ifndef FILESYS_H
#define FILESYS_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <cassert>
#include <boost/filesystem.hpp>
#include "boost/format.hpp"
using namespace boost::filesystem;
using namespace std;

namespace filesys
{
    enum class cause_t
    {
        SAME,
        REMOVED,
        ADDED,
        CONTENT,
        FILE_TO_DIR,
		DIR_TO_FILE,
    };

    string cause_t_str(cause_t aCause, bool aUserFriendly=false);

    struct diff_t 
    {
        path item;
        path item_relative_base;
        cause_t cause;
        shared_ptr<diff_t> parent;
        map<path, shared_ptr<diff_t>> childs;

        path left_base;
        path right_base;

        path left_absolute() { return   path(left_base)+=item_relative_base; }
        path right_absolute() { return path(right_base)+=item_relative_base; }
        path x_absolute(int x) { return x ? right_absolute() :  left_absolute(); }

    };

    shared_ptr<diff_t> diff_tree(path aLeftBase, path aRightBase); 

    path last_element(path aPath);

}


#endif /* FILESYS_H */
