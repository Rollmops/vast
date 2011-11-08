#ifndef ISISMAINWINDOW_HPP
#define ISISMAINWINDOW_HPP

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "qviewercore.hpp"
#include "preferenceDialog.hpp"
#include "nativeimageops.hpp"
#include <CoreUtils/progressfeedback.hpp>
#include "scalingWidget.hpp"
#include "loggingDialog.hpp"

namespace isis
{
namespace viewer
{
namespace widget
{
class PreferencesDialog;
class ScalingWidget;
class LoggingDialog;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow( QViewerCore *core );

	virtual ~MainWindow() {};

	const Ui::vastMainWindow &getUI() const { return m_UI; }
	Ui::vastMainWindow &getUI() { return m_UI; }

	void reloadPluginsToGUI( );
	QLabel *getWorkignInformationLabel() { return m_WorkingInformationLabel; }
	

public Q_SLOTS:
	void showPreferences();
	void openImage();
	void openDir();
	void saveImage();
	void saveImageAs();
	void loadSettings();
	void saveSettings();
	void closeEvent( QCloseEvent * );
	void findGlobalMin();
	void findGlobalMax();
	void spinRadiusChanged(int);
	void showScalingOption();
	void ignoreOrientation(bool);
	void showLoggingDialog();

private:
	Ui::vastMainWindow m_UI;
	QViewerCore *m_ViewerCore;

	QToolBar *m_Toolbar;
	isis::viewer::widget::PreferencesDialog *m_PreferencesDialog;
	isis::viewer::widget::LoggingDialog *m_LoggingDialog;

	widget::ScalingWidget *m_ScalingWidget;
	QSpinBox *m_RadiusSpin;
	QLabel *m_WorkingInformationLabel;
	QPushButton *m_LogButton;
	
	

};


}
}





#endif