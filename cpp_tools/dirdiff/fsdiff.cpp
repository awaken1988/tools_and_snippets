/*
 * fsdiff.cpp
 *
 *  Created on: Jan 2, 2018
 *      Author: martin
 */

#include "fsdiff.h"

namespace fsdiff
{

	static string indent_str(int aLevel)
	{
		string ret;

		for(int i=0; i<aLevel; i++)
			ret += "    ";

		return ret;
	}

	string cause_t_str(cause_t aCause)
	{
		switch(aCause)
		{
			case cause_t::SAME:			return "SAME";
			case cause_t::ADDED:		return "ADDED";
			case cause_t::DELETED:		return "DELETED";
			case cause_t::FILE_TO_DIR:	return "FILE_TO_DIR";
			case cause_t::DIR_TO_FILE:	return "DIR_TO_FILE";
			default: return "UNKNOWN";
		}

	}

	path diff_t::getLastName(idx_t aIdx)
	{
	   path ret;

	   for(auto iElements: fullpath[aIdx]) {
		   ret = iElements;
	   }

	   return ret;
	}

	bool diff_t::isBase()
	{
		if( fullpath[LEFT] == baseDir[LEFT] )
			return true;

		return false;
	}


	int next_debug_id = 1000000;
	static shared_ptr<diff_t> impl_list_dir_rekursive(path aAbsoluteBase, path aOwnPath, shared_ptr<diff_t> aParent)
	{
		shared_ptr<diff_t> ret = make_shared<diff_t>();

		ret->fullpath[diff_t::LEFT] = aOwnPath;
		ret->baseDir[diff_t::LEFT]  = aAbsoluteBase;
		ret->cause = cause_t::SAME;
		ret->parent = aParent;
		ret->debug_id = ++next_debug_id;

		if( !is_directory( aOwnPath ) )
			return ret;

		for(directory_entry iEntry: directory_iterator( ret->fullpath[diff_t::LEFT] ) ) {
			ret->childs.push_back( impl_list_dir_rekursive(aAbsoluteBase, iEntry.path(), ret) );
		}

		return ret;
	}

	shared_ptr<diff_t> list_dir_rekursive(path aAbsoluteBase)
	{
		return impl_list_dir_rekursive(aAbsoluteBase, aAbsoluteBase, nullptr);
	}


	static void impl_copy_diff(shared_ptr<diff_t> aLeft, shared_ptr<diff_t> aRight)
	{
		aLeft->fullpath[diff_t::RIGHT] = aRight->fullpath[diff_t::LEFT];
		aLeft->baseDir[diff_t::RIGHT] = aRight->baseDir[diff_t::LEFT];
	}

	static void impl_set_cause_rekurively(shared_ptr<diff_t> aDiff, cause_t aCause)
	{
		aDiff->cause = aCause;
		for(auto& iChild: aDiff->childs) {
			impl_set_cause_rekurively(iChild, aCause);
		}
	}

	static void impl_move(shared_ptr<diff_t> aDiff, diff_t::idx_t aIdxFrom, diff_t::idx_t aIdxTo, bool aIsRek)
	{
		aDiff->fullpath[aIdxTo] = aDiff->fullpath[aIdxFrom];
		aDiff->fullpath[aIdxFrom] = path();

		aDiff->baseDir[aIdxTo] = aDiff->baseDir[aIdxFrom];
		aDiff->baseDir[aIdxFrom] = path();

		if( aIsRek ) {
			for(auto& iChild: aDiff->childs) {
				impl_move(iChild, aIdxFrom, aIdxTo, aIsRek);
			}
		}
	}

	static shared_ptr<diff_t> impl_compare(shared_ptr<diff_t> aLeft, shared_ptr<diff_t> aRight)
	{
		for(auto& iChild: aLeft->childs) {
			auto right_iter =  find_if(aRight->childs.begin(), aRight->childs.end(), [&iChild](shared_ptr<diff_t>& aDiff) {
				return aDiff->getLastName() == iChild->getLastName();
			});

			const bool isInRight = right_iter != aRight->childs.end();
			const bool isDirLeft = is_directory( iChild->fullpath[diff_t::LEFT] );
			bool copy_right = true;

			if( !isInRight ) {
				iChild->cause = cause_t::DELETED;
				impl_set_cause_rekurively(iChild, iChild->cause);
				continue;
			}

			const bool isDirRight = is_directory( (*right_iter)->fullpath[diff_t::LEFT] );

			if( isDirLeft && !isDirRight ) {
				iChild->cause = cause_t::DIR_TO_FILE;
				impl_set_cause_rekurively(iChild, iChild->cause);
				impl_copy_diff(iChild, *right_iter);
				continue;
			}

			if( !isDirLeft && isDirRight ) {
				iChild->cause = cause_t::FILE_TO_DIR;
				iChild->childs.clear();
				iChild->childs = (*right_iter)->childs;
				for(auto& iChildChild: iChild->childs) {
					iChildChild->parent = iChild;
				}

				impl_set_cause_rekurively(iChild, iChild->cause);
				impl_copy_diff(iChild, *right_iter);
				continue;
			}

			impl_copy_diff(iChild, *right_iter);

			if( isDirLeft ) {
				impl_compare(iChild, *right_iter);
			}
		}

		for(auto& iChild: aRight->childs) {
			auto found =  find_if(aLeft->childs.begin(), aLeft->childs.end(), [&iChild](shared_ptr<diff_t>& aDiff) {
				return aDiff->getLastName() == iChild->getLastName();
			});

			if( found == aLeft->childs.end() ) {
				impl_set_cause_rekurively(iChild, cause_t::ADDED);
				iChild->parent = aLeft;
				impl_move(iChild, diff_t::LEFT, diff_t::RIGHT, true);
				aLeft->childs.push_back( iChild );
			}
		};

	}

	shared_ptr<diff_t> compare(path aAbsoluteLeft, path aAbsoluteRight)
	{
		shared_ptr<diff_t> left = impl_list_dir_rekursive(aAbsoluteLeft, aAbsoluteLeft, nullptr);
		shared_ptr<diff_t> right = impl_list_dir_rekursive(aAbsoluteRight, aAbsoluteRight, nullptr);

		impl_compare(left, right);

		return left;
	}

	void dump(shared_ptr<diff_t> &aTree, int aDepth)
	{
		using namespace std;

		cout<<indent_str(aDepth)<<"* "<<endl;
		cout<<indent_str(aDepth)<<"debug_id="<<aTree->debug_id<<endl;
		cout<<indent_str(aDepth)<<"parent="<<aTree->parent.get()<<endl;
		cout<<indent_str(aDepth)<<"self="<<aTree.get()<<endl;
		cout<<indent_str(aDepth)<<"left  fullpath"<<aTree->fullpath[diff_t::LEFT]<<endl;
		cout<<indent_str(aDepth)<<"right fullpath"<<aTree->fullpath[diff_t::RIGHT]<<endl;
		cout<<indent_str(aDepth)<<"left  baseDir"<<aTree->baseDir[diff_t::LEFT]<<endl;
		cout<<indent_str(aDepth)<<"right baseDir"<<aTree->baseDir[diff_t::RIGHT]<<endl;


		for(auto& iChild: aTree->childs) {
			dump(iChild, aDepth+1);
		}
	}

} /* namespace fsdiff */
