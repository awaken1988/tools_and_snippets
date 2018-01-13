/*
 * fsdiff.h
 *
 *  Created on: Jan 2, 2018
 *      Author: martin
 */

#ifndef FSDIFF_H_
#define FSDIFF_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <cassert>
#include <boost/filesystem.hpp>
#include "boost/format.hpp"

using namespace std;
using namespace boost::filesystem;

namespace fsdiff
{
	enum class cause_t
	{
		SAME,
		ADDED,
		DELETED,
		FILE_TO_DIR,
		DIR_TO_FILE,
	};

	string cause_t_str(cause_t aCause);

	struct diff_t
	{
		enum idx_t {
			LEFT=0,
			RIGHT=1,
		};

		array<path, 2> baseDir;
		array<path, 2> fullpath;
		cause_t cause;

		shared_ptr<diff_t> parent;
		vector<shared_ptr<diff_t>>	childs;

		path getLastName(idx_t aIdx=LEFT);
		bool isBase();

		int debug_id;
	};

	shared_ptr<diff_t> list_dir_rekursive(path aAbsoluteBase);

	shared_ptr<diff_t> compare(path aAbsoluteLeft, path aAbsoluteRight);

	void dump(shared_ptr<diff_t> & aTree, int aDepth=0);

} /* namespace fsdiff */

#endif /* FSDIFF_H_ */
