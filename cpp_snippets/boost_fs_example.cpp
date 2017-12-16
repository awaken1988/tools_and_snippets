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

string indent_str(int aLevel)
{
    string ret;

    for(int i=0; i<aLevel; i++)
        ret += "    ";

    return ret;
}

enum class cause_t
{
    SAME,
    REMOVED,
    ADDED,
    CONTENT,
    TYPE,
};

string cause_t_str(cause_t aCause, bool aUserFriendly=false)
{
    switch(aCause)
    {
    case cause_t::SAME:     return !aUserFriendly ? "SAME"    : "=" ;
    case cause_t::REMOVED:  return !aUserFriendly ? "REMOVED" : "-" ;
    case cause_t::ADDED:    return !aUserFriendly ? "ADDED"   : "+" ;
    case cause_t::CONTENT:  return !aUserFriendly ? "CONTENT" : "~" ;
    case cause_t::TYPE:     return !aUserFriendly ? "TYPE"    : "?" ;
    }

    return "cause_t-unknown";
}

struct diff_t 
{
    path item;
    cause_t cause;
    vector<shared_ptr<diff_t>> childs;

    diff_t()
        : cause(cause_t::SAME) {}

    diff_t(path aItem, cause_t aCause )
        : item(aItem), cause(aCause) {}

   diff_t(path aItem, cause_t aCause, vector<shared_ptr<diff_t>> aChilds )
        : item(aItem), cause(aCause), childs(aChilds) {}

    path last_element() const 
    {   
        path ret;

         for(auto iElements: item) {
            ret = iElements;
        }

        return ret;
    }

    path last_element_slash()
    {
        path ret = last_element();

        if( is_directory(item) ) {
            ret+="/";
        }

        return ret;
    }
};


static vector<path> diff_path(const set<path> aLeft, const set<path> aRight)
{
    vector<path> ret;
    set_difference(aLeft.begin(), aLeft.end(), aRight.begin(), aRight.end(), inserter(ret, ret.begin()));   //TODO: what is an inserter :-O
    return ret;
}

static set<path> dir_list_relative(const path& aPath, const path& aBase)
{
    set<path> ret;

    for(directory_entry& child_path: directory_iterator(aPath)) {
        ret.insert( relative(child_path.path(), aBase) );
    }

    return ret;
}


vector<shared_ptr<diff_t>> iterate_dir_recursively_same(
                             path aBase, 
                             path aSubPath,
                             cause_t aCause)
{
    vector<shared_ptr<diff_t>> ret;
    const path full_path(  path(aBase)+=aSubPath  );

    for(directory_entry& iEntry: directory_iterator(full_path)) {

        const path relat = relative(iEntry.path(), aBase);

        if( is_directory(iEntry.path()) ) {
            auto child_entries = iterate_dir_recursively_same(aBase, relat, aCause);
            ret.push_back( make_shared<diff_t>(relat, aCause, child_entries) );
        } else {
            ret.push_back( make_shared<diff_t>(relat, aCause) );
        }
    }

    return ret;
}

vector<shared_ptr<diff_t>> iterate_dir_recursively(path aLeftBase, 
                             path aRightBase,
                             path aSubPath,
                             cause_t aCause, 
                             int aLevel=0)
{
    const path left(  path(aLeftBase)+=aSubPath  );
    const path right(  path(aRightBase)+=aSubPath  );
    
    vector<shared_ptr<diff_t>> ret;

    if( !exists(left) ) {
        throw "left side path cannot be invalid";
    }

    //create map of dir's child elements
    auto left_set = dir_list_relative(left, aLeftBase);
    auto right_set = dir_list_relative(right, aRightBase);

    //deleted
    for(const path& iLeft: diff_path(left_set, right_set) ) {
        auto diff =  make_shared<diff_t>(iLeft, cause_t::REMOVED);
        const path child = path(aLeftBase)/iLeft;
        if( is_directory( child ) ) {
            diff->childs = iterate_dir_recursively_same(aLeftBase, iLeft, cause_t::REMOVED);
        }
        ret.push_back(diff) ;
    }

    //added
    for(const path& iRight: diff_path(right_set, left_set) ) {
        auto diff =  make_shared<diff_t>(iRight, cause_t::ADDED);
        const path child = path(aRightBase)/iRight;
        if( is_directory( child ) ) {
            diff->childs = iterate_dir_recursively_same(aRightBase, iRight, cause_t::ADDED);
        }
        ret.push_back(diff) ;
    }

    //changed
    vector<path> intersectionOnly;
    set_intersection( right_set.begin(), right_set.end(), left_set.begin(),left_set.end(), inserter(intersectionOnly, intersectionOnly.begin()) );
    for(path iIntersection: intersectionOnly) {
        const path curr_left = path(aLeftBase)+=iIntersection;
        const path curr_right = path(aRightBase)+=iIntersection;
        cause_t cause = cause_t::SAME;

        if( is_directory(curr_left) && !is_directory(curr_right) ) {
            cause = cause_t::TYPE;
        }
        else if( !is_directory(curr_left) && is_directory(curr_right) ) {
            cause = cause_t::TYPE;
        }
        else if( is_regular_file(curr_left) && file_size(curr_left) != file_size(curr_right) ) {
            cause = cause_t::CONTENT;
        }
        //TODO: compare file by date, btrfs-checksum...

        cout<<"-"<<endl;
        if( cause_t::SAME != cause ) {
            auto diff_entry = make_shared<diff_t>(iIntersection, cause);
            ret.push_back( diff_entry );
        }
        else if ( is_directory(curr_left) ) {
            auto child_diff = iterate_dir_recursively(aLeftBase, aRightBase, iIntersection, cause_t::SAME, aLevel+1 );   
            if( 0 == child_diff.size()  )
                continue;
            ret.push_back( make_shared<diff_t>(iIntersection, cause, child_diff) ); //try out move here
        }
    }

    return ret;
}

void print_dir_recursive( vector<shared_ptr<diff_t>> aDiffList, int aDepth=0 )
{
    using boost::format;
    using boost::io::group;

    for( auto iParent: aDiffList )
    {
        cout<< indent_str(aDepth)
            << format("%1%:  %2%")
            % cause_t_str(iParent->cause, true) 
            % iParent->last_element_slash()
            <<endl;
        
        if( is_directory(iParent->item) ) {
            cout << indent_str(aDepth) << "\\---"<<endl;;
        }
        print_dir_recursive(iParent->childs, aDepth+1);
    }
}

int main(int argc, char* argv[])
{
    path left(argv[1]);
    path right(argv[2]);

    auto diff_list = iterate_dir_recursively(left, right, path(), cause_t::SAME);
    print_dir_recursive(diff_list);



    return 0;
}