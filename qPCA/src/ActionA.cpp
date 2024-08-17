// Example of a plugin action

#include "ccMainAppInterface.h"
#include <qmessagebox.h>
#include <Eigen/Core>

namespace Example
{
	// This is an example of an action's method called when the corresponding action
	// is triggered (i.e. the corresponding icon or menu entry is clicked in CC's
	// main interface). You can access most of CC's components (database,
	// 3D views, console, etc.) via the 'appInterface' variable.
	void performActionA( ccMainAppInterface *appInterface )
	{
		assert(m_app);
		if (!m_app)
			return;
		if ( appInterface == nullptr )
		{
			// The application interface should have already been initialized when the plugin is loaded
			Q_ASSERT( false );
			
			return;
		}
		
		/*** HERE STARTS THE ACTION ***/
	
		// Put your code here
		appInterface->dispToConsole( "[PCAPlugin] welcome use PCA plugin by xxx!", ccMainAppInterface::STD_CONSOLE_MESSAGE );
		QMessageBox::information(nullptr, "info", "welcome use PCA plugin");
		// --> you may want to start by asking for parameters (with a custom dialog, etc.)
	
		CCVector3f eigenValuesPCA;
		Eigen:
	}
}
