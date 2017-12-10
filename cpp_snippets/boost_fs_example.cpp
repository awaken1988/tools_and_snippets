#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <cassert>
#include <boost/filesystem.hpp>
using namespace boost::filesystem;
using namespace std;

string indent_str(int aLevel)
{
    string ret;

    for(int i=0; i<aLevel; i++)
        ret += "\t";

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

struct diff_t 
{
    path item;
    vector<unique_ptr<diff_t>> childs;
    cause_t cause;
};

void iterate_dir_recursively(path aLeftBase, 
                             path aRightBase,
                             path aSubPath,
                             cause_t aCause, 
                             int aLevel=0)
{
    const path left(  path(aLeftBase)+=aSubPath  );
    const path right(  path(aRightBase)+=aSubPath  );


    if( !exists(left) ) {
        throw "left side path cannot be invalid";
    }

    set<path> left_set, right_set;

    for(directory_entry& child_path: directory_iterator(left)) {
        left_set.insert( relative(child_path.path(), aLeftBase) );
    }
    for(directory_entry& child_path: directory_iterator(right)) {
        right_set.insert( relative(child_path.path(), aRightBase) );
    }

    vector<path> leftOnly;
    set_difference( left_set.begin(),left_set.end(), right_set.begin(), right_set.end(), inserter(leftOnly, leftOnly.begin()) );
    for(path iLeft: leftOnly) {
        const path curr = path(aLeftBase)+=iLeft;
        cout<<indent_str(aLevel)<<"--"<<iLeft<<endl;
    }

    vector<path> rightOnly;
    set_difference( right_set.begin(), right_set.end(), left_set.begin(),left_set.end(), inserter(rightOnly, rightOnly.begin()) );
    for(path iRight: rightOnly) {
        const path curr = path(aRightBase)+=iRight;
        cout<<indent_str(aLevel)<<"++"<<iRight<<endl;
    }

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

        if( cause_t::SAME == cause && is_directory(curr_left) ) {
            iterate_dir_recursively(aLeftBase, aRightBase, iIntersection, cause_t::SAME, aLevel+1 );
        } else if( cause_t::SAME != cause ){
            cout<<indent_str(aLevel)<<iIntersection<<endl;
        }
    }






}



//void iterate_dir_recursively(path aLeftBase, 
//                             path aRightBase,
//                             path aSubPath, 
//                             int aLevel=0)
//{
//    const path curr_path( path(aLeftBase)+=aSubPath );
//    const string indent = indent_str(aLevel);
//    const bool is_dir = is_directory(curr_path);
//    const string file_or_dir = is_dir ? "+" : "*";
//
//    cout<<indent<<file_or_dir<<aSubPath<<endl;
//
//    if( is_dir ) {
//        for(directory_entry& next_path: directory_iterator(curr_path)) {
//            const path relative_path = relative(next_path.path(), aLeftBase); 
//
//            iterate_dir_recursively(aLeftBase, relative_path, aLevel+1);
//        }
//    }
//}




int main(int argc, char* argv[])
{
    path left(argv[1]);
    path right(argv[2]);

    iterate_dir_recursively(left, right, path(), cause_t::SAME);


}