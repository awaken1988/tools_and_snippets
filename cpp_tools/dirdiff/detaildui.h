/*
 * detaildui.h
 *
 *  Created on: Jan 13, 2018
 *      Author: martin
 */

#ifndef DETAILDUI_H_
#define DETAILDUI_H_

#include <QWidget>
#include "fsdiff.h"

namespace detailgui
{
	void show(fsdiff::diff_t* aDiff, QWidget* aLeft, QWidget* aRight);
}



#endif /* DETAILDUI_H_ */
