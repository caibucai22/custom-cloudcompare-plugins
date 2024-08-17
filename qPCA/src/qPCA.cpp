// First:
//	Replace all occurrences of 'ExamplePlugin' by your own plugin class name in this file.
//	This includes the resource path to info.json in the constructor.

// Second:
//	Open ExamplePlugin.qrc, change the "prefix" and the icon filename for your plugin.
//	Change the name of the file to <yourPluginName>.qrc

// Third:
//	Open the info.json file and fill in the information about the plugin.
//	 "type" should be one of: "Standard", "GL", or "I/O" (required)
//	 "name" is the name of the plugin (required)
//	 "icon" is the Qt resource path to the plugin's icon (from the .qrc file)
//	 "description" is used as a tootip if the plugin has actions and is displayed in the plugin dialog
//	 "authors", "maintainers", and "references" show up in the plugin dialog as well

#include "qPCA.h"

#include <QtGui>
#include <qmessagebox.h>
#include <QApplication>
#include <QMainWindow>

//Dialog
#include "ccPCADlg.h"



#include <ccPointCloud.h>
#include <ccProgressDialog.h>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Dense>

#include "ActionA.h"

#include <iostream>

static ccMainAppInterface* s_app = nullptr;

// Default constructor:
//	- pass the Qt resource path to the info.json file (from <yourPluginName>.qrc file) 
//  - constructor should mainly be used to initialize actions and other members
qPCA::qPCA( QObject *parent )
	: QObject( parent )
	, ccStdPluginInterface( ":/CC/plugin/qPCA/info.json" )
	, m_action( nullptr )
{
	s_app = m_app;
}

// This method should enable or disable your plugin actions
// depending on the currently selected entities ('selectedEntities').
void qPCA::onNewSelection( const ccHObject::Container &selectedEntities )
{
	if (m_action)
		m_action->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
	
}

// This method returns all the 'actions' your plugin can perform.
// getActions() will be called only once, when plugin is loaded.
QList<QAction *> qPCA::getActions()
{
	// default action (if it has not been already created, this is the moment to do it)
	if ( !m_action )
	{
		// Here we use the default plugin name, description, and icon,
		// but each action should have its own.
		m_action = new QAction( getName(), this );
		m_action->setToolTip( getDescription() );
		m_action->setIcon( getIcon() );
		
		// Connect appropriate signal
		connect( m_action, &QAction::triggered, this, &qPCA::doAction);
	}

	return { m_action };
}


static bool axis_x_checked = true;
static bool axis_y_checked = false;
static bool axis_z_checked = false;
void qPCA::doAction()
{
	assert(m_app);
	if (!m_app)
		return;
	m_app->dispToConsole("[qPCA] welcome use PCA plugin by xxx!", ccMainAppInterface::STD_CONSOLE_MESSAGE);
	QMessageBox::information(nullptr, "info", "welcome use PCA plugin");

	const ccHObject::Container& selectedEntities = m_app->getSelectedEntities();
	size_t selNum = selectedEntities.size();
	if (selNum != 1)
	{
		ccLog::Error("[qPCA] Select only one cloud!");
		return;
	}

	ccHObject* ent = selectedEntities[0];
	assert(ent);
	if (!ent || !ent->isA(CC_TYPES::POINT_CLOUD))
	{
		ccLog::Error("[qPCA] Select a real point cloud!");
		return;
	}

	ccPointCloud* pc = static_cast<ccPointCloud*>(ent);

	// input cloud
	CCVector3 bbMin, bbMax;
	pc->getBoundingBox(bbMin, bbMax);
	/*CCVector3 diff = bbMax - bbMin;
	float scale = std::max(std::max(diff[0], diff[1]), diff[2]);*/

	ccPCADlg pcaDlg(m_app->getMainWindow());
	if (!pcaDlg.exec())
	{
		return;
	}
	axis_x_checked = pcaDlg.radioButton->isChecked();
	axis_y_checked = pcaDlg.radioButton_2->isChecked();
	axis_z_checked = pcaDlg.radioButton_3->isChecked();
	

	Eigen::Vector3f eigenValuesPCA;
	Eigen::Matrix3f eigenVectorsPCA;
	Eigen::Vector3f pcaCentroid;
	ccHObject* group = executePCA(pc,eigenValuesPCA,eigenVectorsPCA, pcaCentroid,false);

	if (group)
	{
		m_app->addToDB(group);
		m_app->refreshAll();
	}
}

ccHObject* qPCA::executePCA(ccPointCloud* ccPC,
	Eigen::Vector3f& eigenValuesPCA,
	Eigen::Matrix3f& eigenVectorsPCA,
	Eigen::Vector3f& pcaCentroid,
	bool silent)
{
	ccProgressDialog* pDlg = nullptr;
	//if (!silent)
	//{
	//	pDlg = new ccProgressDialog(false, s_app ? s_app->getMainWindow() : nullptr);
	//	pDlg->setWindowTitle("PCA computing");
	//	pDlg->setMethodTitle(tr("please wait"));
	//	pDlg->setRange(0, 0); // infinite loop
	//	pDlg->show();
	//}
	//QApplication::processEvents();

	ccHObject* group = nullptr;
	const CCVector3d& globalShift = ccPC->getGlobalShift();
	double globalScale = ccPC->getGlobalScale();

	auto toEigen = [](const CCVector3* vec) {
		return Eigen::Vector3f(vec->x, vec->y, vec->z);
	};
	pcaCentroid.setZero();
	for (unsigned i = 0; i < ccPC->size(); ++i)
	{
		const CCVector3* point = ccPC->getPoint(i);
		Eigen::Vector3f eigenPoint(point->x, point->y, point->z);
		pcaCentroid += eigenPoint;
	}
	pcaCentroid /= static_cast<float>(ccPC->size());

	Eigen::Matrix3f covarianceMatrix = Eigen::Matrix3f::Zero();
	for (unsigned i = 0; i < ccPC->size(); ++i)
	{
		Eigen::Vector3f diff = (toEigen(ccPC->getPoint(i))) - pcaCentroid;
		covarianceMatrix += diff * diff.transpose();
	}
	covarianceMatrix /= static_cast<float>(ccPC->size());

	// 进行 PCA：求解特征值和特征向量
	Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covarianceMatrix);
	eigenValuesPCA = solver.eigenvalues();   // 返回特征值
	eigenVectorsPCA = solver.eigenvectors(); // 返回特征向量

	// log
	Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
	std::stringstream vectorStream, matrixStream;
	vectorStream << pcaCentroid.format(CleanFmt);
	m_app->dispToConsole("[qPCA] pca center", ccMainAppInterface::STD_CONSOLE_MESSAGE);
	m_app->dispToConsole(QString::fromStdString(vectorStream.str()), ccMainAppInterface::STD_CONSOLE_MESSAGE);

	vectorStream.str("");
	m_app->dispToConsole("[qPCA] eigen values", ccMainAppInterface::STD_CONSOLE_MESSAGE);
	vectorStream << eigenValuesPCA.format(CleanFmt);
	matrixStream << eigenVectorsPCA.format(CleanFmt);

	m_app->dispToConsole(QString::fromStdString(vectorStream.str()), ccMainAppInterface::STD_CONSOLE_MESSAGE);
	m_app->dispToConsole("[qPCA] eigen vectors sorted by eigen value in descending order", ccMainAppInterface::STD_CONSOLE_MESSAGE);
	m_app->dispToConsole(QString::fromStdString(matrixStream.str()), ccMainAppInterface::STD_CONSOLE_MESSAGE);
	//m_app->forceConsoleDisplay();



	// 将点云主方向转换到 x y z 轴上
	char axis = axis_y_checked ? 'y' : (axis_z_checked ? 'z' : 'x');
	m_app->dispToConsole(QString::fromStdString("[qPCA] frist component 2 axis "+std::tolower(axis)), ccMainAppInterface::STD_CONSOLE_MESSAGE);
	//char axis = 'x'; //通过对话框获取 默认
	Eigen::Matrix4f rotationMatrix = Eigen::Matrix4f::Identity();
	Eigen::Matrix3f tmp;
	switch (axis)
	{
	case 'x':
		rotationMatrix.block<3, 3>(0, 0) = eigenVectorsPCA.transpose(); // x y z
		break;
	case 'y':
		tmp = eigenVectorsPCA;
		tmp.col(0).swap(tmp.col(1));
		rotationMatrix.block<3, 3>(0, 0) = tmp.transpose(); // y x z
		break;
	case 'z':
		tmp = eigenVectorsPCA;
		tmp.col(0).swap(tmp.col(2));
		rotationMatrix.block<3, 3>(0, 0) = tmp.transpose(); // z x y
		break;
	default:
		break;
	}
	matrixStream.str("");
	matrixStream << rotationMatrix.format(CleanFmt);
	m_app->dispToConsole(QString::fromStdString(matrixStream.str()), ccMainAppInterface::STD_CONSOLE_MESSAGE);

	rotationMatrix.block<3, 1>(0, 3) = -1.0f * ((axis_x_checked ? eigenVectorsPCA.transpose() : tmp.transpose()) * pcaCentroid);

	matrixStream.str("");
	matrixStream << rotationMatrix.format(CleanFmt);
	m_app->dispToConsole(QString::fromStdString(matrixStream.str()), ccMainAppInterface::STD_CONSOLE_MESSAGE);

	ccPointCloud* firstComponent = new ccPointCloud(
		QString("first  component - projecting to (%1) plane ").arg((axis_y_checked ? "xz" : (axis_z_checked ? "xy" : "yz")))
	);
	ccPointCloud* secondComponent = new ccPointCloud(
		QString("second component - projecting to (%1) plane ").arg((axis_y_checked ? "yz" : (axis_z_checked ? "zy" : "xz")))
	);
	ccPointCloud* thirdComponent = new ccPointCloud(
		QString("third  component - projecting to (%1) plane ").arg((axis_y_checked ? "yx" : (axis_z_checked ? "zx" : "xy")))
	); // 主成分

	ccPointCloud* stdAxisCloud = new ccPointCloud("2stdAxisCloud");

	if (!firstComponent->reserve(static_cast<unsigned>(ccPC->size())))
	{
		ccLog::Error("[qPCA] Not enough memory!");
		delete firstComponent;
		return nullptr;
	}
	if (!secondComponent->reserve(static_cast<unsigned>(ccPC->size())))
	{
		ccLog::Error("[qPCA] Not enough memory!");
		delete secondComponent;
		return nullptr;
	}
	if (!thirdComponent->reserve(static_cast<unsigned>(ccPC->size())))
	{
		ccLog::Error("[qPCA] Not enough memory!");
		delete thirdComponent;
		return nullptr;
	}
	if (!stdAxisCloud->reserve(static_cast<unsigned>(ccPC->size())))
	{
		ccLog::Error("[qPCA] Not enough memory!");
		delete stdAxisCloud;
		return nullptr;
	}

	// 遍历每个点并应用旋转矩阵
	std::stringstream pointStream;
	for (unsigned i = 0; i < ccPC->size(); ++i)
	{
		pointStream.str("");
		CCVector3* point = const_cast<CCVector3*>(ccPC->getPoint(i));

		// 将 CCVector3 转换为 Eigen::Vector3f
		Eigen::Vector4f eigenPoint(point->x, point->y, point->z, 1.0f);

		// 旋转点
		Eigen::Vector4f rotatedPoint = rotationMatrix * eigenPoint;

		// 将结果写回 CCVector3
		/*point->x = rotatedPoint.x();
		point->y = rotatedPoint.y();
		point->z = rotatedPoint.z();*/
		//pointStream << point->x << "," << point->y << "," << point->z;
		//m_app->dispToConsole(QString::fromStdString(pointStream.str()), ccMainAppInterface::STD_CONSOLE_MESSAGE);

		stdAxisCloud->addPoint({ rotatedPoint[0],rotatedPoint[1],rotatedPoint[2] });
		
		if (axis_y_checked) // align to y // y x z
		{
			firstComponent->addPoint({ rotatedPoint[0],0.0f,rotatedPoint[2] });
			secondComponent->addPoint({ 0.0f,rotatedPoint[1],rotatedPoint[2] });
			thirdComponent->addPoint({ rotatedPoint[0],rotatedPoint[1],0.0f });
		}
		else if (axis_x_checked) // align to x // x y z
		{
			firstComponent->addPoint({ 0.0f,rotatedPoint[1],rotatedPoint[2] });
			secondComponent->addPoint({ rotatedPoint[0],0.0f,rotatedPoint[2] });
			thirdComponent->addPoint({ rotatedPoint[0],rotatedPoint[1],0.0f });
		}
		else if(axis_z_checked) // align to  z // z x y
		{
			firstComponent->addPoint({ rotatedPoint[0],rotatedPoint[1],0.0f });
			secondComponent->addPoint({ 0.0f,rotatedPoint[1],rotatedPoint[2] });
			thirdComponent->addPoint({ rotatedPoint[0],0.0f,rotatedPoint[2] });
		}
		else
		{
			ccLog::Error("[qPCA] axis error");
			return nullptr;
		}


	}
	// 更新点云
	//ccPC->invalidateBoundingBox();
	//ccPC->setVisible(false);

	// 设置 主成分 颜色 可视化
	for (auto pcShape : { stdAxisCloud ,firstComponent,secondComponent,thirdComponent })
	{
		ccColor::Rgb col = ccColor::Generator::Random();
		pcShape->setColor(col);
		pcShape->showSF(false);
		pcShape->showColors(true);
		pcShape->showNormals(true);
		pcShape->setVisible(true);
	}


	// 计算投影 各个方向 主成分 已经变换到标准坐标系下, 直接坐标赋0
	//ccPointCloud firstComponent, secondComponent, thirdComponent; // 合并到上面的循环完成
	if (!group)
	{
		group = new ccHObject(QString("PCA processed - align to %1 axis (%2)").arg((axis_y_checked?"y":(axis_z_checked?"z":"x")), ccPC->getName()));
	}
	if (group)
	{
		group->addChild(stdAxisCloud);
		group->addChild(firstComponent);
		group->addChild(secondComponent);
		group->addChild(thirdComponent);
	}

	/*if (pDlg)
	{
		pDlg->hide();
		delete pDlg;
	}*/

	return group;
}
