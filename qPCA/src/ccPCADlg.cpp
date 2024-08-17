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

#include "ccPCADlg.h"
#include <QButtonGroup>


static bool axis_x_checeked  = true;
static bool axis_y_checeked = false;	
static bool axis_z_checeked = false;	

ccPCADlg::ccPCADlg(QWidget* parent)
	: QDialog(parent)
	, Ui::PCADialog()
{
	setupUi(this);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &ccPCADlg::saveSettings);
	// 创建一个 QButtonGroup 逻辑上保证只有一个被选中
	QButtonGroup* buttonGroup = new QButtonGroup(this);
	buttonGroup->addButton(radioButton);
	buttonGroup->addButton(radioButton_2);
	buttonGroup->addButton(radioButton_3);
	radioButton->setChecked(true); // default x
}


void ccPCADlg::saveSettings()
{
	axis_x_checeked = radioButton->isChecked();
	axis_y_checeked = radioButton_2->isChecked();
	axis_z_checeked = radioButton_3->isChecked();
}
