#ifndef IMAGESTACKWIDGET_HPP
#define IMAGESTACKWIDGET_HPP

#include "ui_imageStackWidget.h"
#include "qviewercore.hpp"

namespace isis
{
namespace viewer
{
namespace widget
{



class ImageStackWidget : public QWidget
{
	Q_OBJECT
public:
	ImageStackWidget( QWidget *parent, QViewerCore *core );

	Ui::imageStackWidget &getInterface() { return m_Interface; }

	void updateImageStack();
private:
	ViewerCoreBase *m_Core;
	Ui::imageStackWidget m_Interface;

};


}
}
}



#endif