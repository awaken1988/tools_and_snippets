#include "filesys.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <cassert>
#include <array>
#include <boost/filesystem.hpp>
#include "boost/format.hpp"
using namespace boost::filesystem;
using namespace std;


/*
example

int man(int argc, char* argv[])
{
    path left(argv[1]);
    path right(argv[2]);

    auto diff_list = iterate_dir_recursively(left, right, path(), cause_t::SAME);
    print_dir_recursive(diff_list);



    return 0;
}
*/


namespace filesys
{
    static string indent_str(int aLevel)
    {
        string ret;

        for(int i=0; i<aLevel; i++)
            ret += "    ";

        return ret;
    }

    
    string cause_t_str(cause_t aCause, bool aUserFriendly)
    {
        switch(aCause)
        {
        case cause_t::SAME:     		return !aUserFriendly ? "SAME"    : "=" ;
        case cause_t::REMOVED:  		return !aUserFriendly ? "REMOVED" : "-" ;
        case cause_t::ADDED:    		return !aUserFriendly ? "ADDED"   : "+" ;
        case cause_t::CONTENT:  		return !aUserFriendly ? "CONTENT" : "~" ;
        case cause_t::FILE_TO_DIR:  	return !aUserFriendly ? "FILE_TO_DIR" : " " ;
        case cause_t::DIR_TO_FILE:		return !aUserFriendly ? "DIR_TO_FILE"    : " " ;

        }

        return "cause_t-unknown";
    }

    path last_element(path aPath)
    {   
        path ret;

        for(auto iElements: aPath) {
            ret = iElements;
        }

        return ret;
    }

 /*    static vector<path> diff_path(const set<path> aLeft, const set<path> aRight)
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


    static vector<shared_ptr<diff_t>> iterate_dir_recursively_same(
                                path aBase, 
                                path aSubPath,
                                cause_t aCause,
                                shared_ptr<diff_t> aParent)
    {
        vector<shared_ptr<diff_t>> ret;
        const path full_path(  path(aBase)+=aSubPath  );

        for(directory_entry& iEntry: directory_iterator(full_path)) {

            const path relat = relative(iEntry.path(), aBase);

            auto curr_diff = make_shared<diff_t>(relat, aCause, aParent); 

            if( is_directory(iEntry.path()) ) {
                curr_diff->childs = iterate_dir_recursively_same(aBase, relat, aCause, curr_diff);
            }

            ret.push_back( curr_diff );  
        }
        return ret;
    }

    vector<shared_ptr<diff_t>> iterate_dir_recursively(path aLeftBase, 
                                path aRightBase,
                                path aSubPath, 
                                shared_ptr<diff_t> aParent)
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
            auto diff =  make_shared<diff_t>(iLeft, cause_t::REMOVED, aParent);
            const path child = path(aLeftBase)/iLeft;
            if( is_directory( child ) ) {
                diff->childs = iterate_dir_recursively_same(aLeftBase, iLeft, cause_t::REMOVED, diff);
            }
            ret.push_back(diff) ;
        }

        //added
        for(const path& iRight: diff_path(right_set, left_set) ) {
            auto diff =  make_shared<diff_t>(iRight, cause_t::ADDED, aParent);
            const path child = path(aRightBase)/iRight;
            if( is_directory( child ) ) {
                diff->childs = iterate_dir_recursively_same(aRightBase, iRight, cause_t::ADDED, diff);
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

            auto diff_entry = make_shared<diff_t>(iIntersection, cause, aParent);

            if ( is_directory(curr_left) &&  cause_t::SAME == cause ) {
                auto child_diff = iterate_dir_recursively(aLeftBase, aRightBase, iIntersection, diff_entry );   
                if( child_diff.size() > 0) {
                    diff_entry->childs = child_diff;
                }
            }

            ret.push_back(diff_entry);
        }

        return ret;
    }
 */
 /*    void print_dir_recursive( vector<shared_ptr<diff_t>> aDiffList, int aDepth )
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
 */


    static void create_dir_list_rek(  
        path aLeftBase, 
        path aRightBase,
        path aSubPath,
        shared_ptr<diff_t>& aParent,
        int aLevel=0) 
    {
        const path left(  path(aLeftBase)+=aSubPath );
        const path right(  path(aRightBase)+=aSubPath  );
        const vector<path> sides{ left, right };
        array<map<path, shared_ptr<diff_t>>, 2> sides_childs;

        const int sides_start = cause_t::ADDED == aParent->cause || cause_t::FILE_TO_DIR == aParent->cause
        	? 1 : 0;
        const int sides_end   = cause_t::REMOVED == aParent->cause || cause_t::DIR_TO_FILE == aParent->cause
        	? 1 : 2;
        //collect list

        for(int iSide=sides_start; iSide<sides_end; iSide++) {
        	bool isLeft = 0 == iSide;
            for(directory_entry iEntry: directory_iterator(sides[iSide]) ) {
                const path left(  path(aLeftBase)+=iEntry.path() );
                const path right(  path(aRightBase)+=iEntry.path()  );
                const path curr_base = isLeft ? aLeftBase : aRightBase;

                auto next_item = make_shared<diff_t>();
                next_item->item = last_element(iEntry.path());
                next_item->item_relative_base = relative(iEntry.path(), curr_base);
                next_item->left_base = aLeftBase;
                next_item->right_base = aRightBase;
                next_item->cause == cause_t::SAME;

                sides_childs[iSide][next_item->item] = next_item;
                //cout<<indent_str(aLevel)<<next_item->item<<"        :           :::::"<< next_item->item_relative_base<<endl;
            }
        }

        //create difflist
        set<path> used;
        for(int iSide=sides_start; iSide<sides_end; iSide++) {
        	bool isLeft = 0 == iSide;
        	auto& otherSide = sides_childs[isLeft ? 1 : 0];
            for( auto iCurrSide: sides_childs[iSide] ) {

            	if( used.find( iCurrSide.second->item) != used.end() )
            		continue;

            	bool isIncluded = otherSide.find(iCurrSide.first) != otherSide.end();
            	bool isDir = is_directory( iCurrSide.second->x_absolute(iSide) );
            	bool isOtherDir = isIncluded && is_directory(otherSide.find(iCurrSide.first)->second->x_absolute(!iSide));

            	if( cause_t::SAME == aParent->cause ) {
					if( isLeft && !isIncluded  ) {
						iCurrSide.second->cause = cause_t::REMOVED;
					}
					else if( !isLeft && !isIncluded  ) {
						iCurrSide.second->cause = cause_t::ADDED;
					}
					else if( isDir && !isOtherDir) {
						iCurrSide.second->cause = isLeft ? cause_t::DIR_TO_FILE : cause_t::FILE_TO_DIR;
					}
					else if( !isDir && isOtherDir) {
						iCurrSide.second->cause = isLeft ? cause_t::FILE_TO_DIR : cause_t::DIR_TO_FILE;
					}
            	}
            	else {
            		iCurrSide.second->cause = aParent->cause;
            	}

            	used.insert( iCurrSide.second->item );

            	aParent->childs[iCurrSide.first] = iCurrSide.second;
            	cout<<indent_str(aLevel)<<aParent->childs[iCurrSide.first]->item<<" "<<cause_t_str(aParent->childs[iCurrSide.first]->cause)<<endl;

            	if( isDir ) {
            		create_dir_list_rek(aLeftBase,
						aRightBase,
						iCurrSide.second->item_relative_base,
						iCurrSide.second,
						aLevel+1);
            	}

            }
        }

    }


    shared_ptr<diff_t> diff_tree(path aLeftBase, path aRightBase)
    {
        auto ret = make_shared<diff_t>();

        ret->left_base = aLeftBase;
        ret->right_base = aRightBase;

        create_dir_list_rek(aLeftBase, aRightBase, "", ret);

        return ret;
    }



}

