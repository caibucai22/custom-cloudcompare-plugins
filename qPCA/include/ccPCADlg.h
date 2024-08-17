//##########################################################################
//#                                                                        #
//#                    CLOUDCOMPARE PLUGIN: qRANSAC_SD                     #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 or later of the License.      #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                  COPYRIGHT: Daniel Girardeau-Montaut                   #
//#                                                                        #
//##########################################################################

#ifndef CC_PCA_DLG_HEADER
#define CC_PCA_DLG_HEADER

#include "ui_pcaDlg.h"

//! Dialog for qRansacSD plugin
class ccPCADlg : public QDialog, public Ui::PCADialog
{
	Q_OBJECT

public:

	//! Default constructor
	explicit ccPCADlg(QWidget* parent = nullptr);


protected:

	//! Saves (temporarily) the dialog parameters on acceptation
	void saveSettings();

};

#endif
