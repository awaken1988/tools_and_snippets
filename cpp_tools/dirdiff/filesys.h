#ifndef FILESYS_H
#define FILESYS_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <set>
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
        TYPE,
    };

    string cause_t_str(cause_t aCause, bool aUserFriendly=false);

    struct diff_t 
    {
        path item;
        cause_t cause;
        shared_ptr<diff_t> parent;
        vector<shared_ptr<diff_t>> childs;

        diff_t();
        diff_t(path aItem, cause_t aCause, shared_ptr<diff_t> aParent );
        diff_t(path aItem, cause_t aCause, shared_ptr<diff_t> aParent, vector<shared_ptr<diff_t>> aChilds );

        path last_element() const;
        path last_element_slash();

    };


    vector<shared_ptr<diff_t>> iterate_dir_recursively(
        path aLeftBase, 
        path aRightBase,
        path aSubPath, 
        shared_ptr<diff_t> aParent);

    void print_dir_recursive( vector<shared_ptr<diff_t>> aDiffList, int aDepth=0 );
}


#endif /* FILESYS_H */
