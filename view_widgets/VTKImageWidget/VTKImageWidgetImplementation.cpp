/****************************************************************
 *
 * <Copyright information>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, Samuel Eckermann, Hans-Christian Heinz
 *
 * VTKImageWidgetImplementation.cpp
 *
 * Description:
 *
 *  Created on: Feb 28, 2012
 ******************************************************************/

#include "VTKImageWidgetImplementation.hpp"

#include "CoreUtils/singletons.hpp"



namespace isis
{
namespace viewer
{
namespace widget
{


VTKImageWidgetImplementation::VTKImageWidgetImplementation()
	: m_RenderWindow( vtkRenderWindow::New() ),
	  m_Renderer( vtkRenderer::New() ),
	  m_Actor( vtkActor::New() ),
	  m_CursorMapper( vtkPolyDataMapper::New() ),
	  m_Cursor( vtkCursor3D::New() )
{
}

VTKImageWidgetImplementation::VTKImageWidgetImplementation ( QViewerCore *core, QWidget *parent, PlaneOrientation orientation )
	: QVTKWidget( parent ),
	  m_Layout( new QVBoxLayout( parent ) ),
	  m_RenderWindow( vtkRenderWindow::New() ),
	  m_Renderer( vtkRenderer::New() )
{
	setup( core, parent, orientation );

	commonInit();
}

void VTKImageWidgetImplementation::setup ( QViewerCore *core, QWidget *parent, PlaneOrientation orientation )
{
	WidgetInterface::setup( core, parent, orientation );
	setParent( parent );
	m_Layout = new QVBoxLayout( parent );
	commonInit();
	dynamic_cast<OptionWidget*>( getWidgetEnsemble()->getOptionWidget() )->setWidget(this);
}


void VTKImageWidgetImplementation::paintEvent ( QPaintEvent *event )
{
	BOOST_FOREACH( ComponentsMapType::reference component, m_VTKImageComponentsMap ) {
		const bool isVisible = component.first->getImageProperties().isVisible;
		component.second.opacityFunction->RemoveAllPoints();
		component.second.colorFunction->RemoveAllPoints();

		for( unsigned short ci = 0; ci < 256; ci++ ) {
			component.second.colorFunction->AddRGBPoint( ci, QColor( component.first->getImageProperties().colorMap[ci] ).redF(),
					QColor( component.first->getImageProperties().colorMap[ci] ).greenF(),
					QColor( component.first->getImageProperties().colorMap[ci] ).blueF() );
		}

		if( component.first->getImageProperties().imageType == ImageHolder::statistical_image ) {
			for( unsigned short ci = 0 ; ci < 256; ci++ ) {
				component.second.opacityFunction->AddPoint( ci, component.first->getImageProperties().alphaMap[ci] * component.first->getImageProperties().opacity * isVisible );
			}
		} else {
			component.second.opacityFunction->AddPoint( 0, 0 );
			component.second.opacityFunction->AddPoint( 1, m_OpacityGradientFactor * component.first->getImageProperties().opacity * isVisible );
			component.second.opacityFunction->AddPoint( 256, component.first->getImageProperties().opacity * isVisible );
		}
	}
	QVTKWidget::paintEvent( event );
}

void VTKImageWidgetImplementation::wheelEvent ( QWheelEvent */*e*/ )
{
	//     QVTKWidget::wheelEvent ( e );
}

void VTKImageWidgetImplementation::commonInit()
{
	//  m_OptionWidget = new OptionWidget( this, m_ViewerCore );
	m_Layout->addWidget( this );
	m_Layout->setMargin( 0 );
	connect( m_ViewerCore, SIGNAL( emitUpdateScene( ) ), this, SLOT( updateScene( ) ) );
	connect( m_ViewerCore, SIGNAL( emitPhysicalCoordsChanged( util::fvector4 ) ), this, SLOT( lookAtPhysicalCoords( util::fvector4 ) ) );
	connect( m_ViewerCore, SIGNAL( emitZoomChanged( float ) ), this, SLOT( setZoom( float ) ) );
	connect( m_ViewerCore, SIGNAL( emitSetEnableCrosshair( bool ) ), this, SLOT( setEnableCrosshair( bool ) ) );
	m_ViewerCore->emitImageContentChanged.connect( boost::bind( &VTKImageWidgetImplementation::reloadImage, this, _1 ) );
	setFocus();
	SetRenderWindow( m_RenderWindow );

	
	
	m_RenderWindow->AddRenderer( m_Renderer );
	m_Renderer->SetBackground( 0.1, 0.2, 0.4 );
	m_OpacityGradientFactor = 0;

	if( m_ViewerCore->getSettings()->getPropertyAs<bool>( "showCrosshair" ) ) {
		m_Cursor->AllOn();
		m_Cursor->OutlineOff();
	} else {
		m_Cursor->AllOff();
	}

	m_Cursor->Update();
	m_CursorMapper->SetInputConnection( m_Cursor->GetOutputPort() );
	m_Actor->SetMapper( m_CursorMapper );
	m_Renderer->AddActor( m_Actor );
}

void VTKImageWidgetImplementation::updateScene()
{
	update();
}

void VTKImageWidgetImplementation::setZoom ( float /*zoom*/ )
{

}

void VTKImageWidgetImplementation::setEnableCrosshair ( bool enable )
{
	if( enable ) {
		m_Cursor->AllOn();
	} else {
		m_Cursor->AllOff();
	}

	m_Cursor->OutlineOff();
	m_RenderWindow->Render();
}

void VTKImageWidgetImplementation::reloadImage ( const ImageHolder::Pointer image )
{
	const ComponentsMapType::iterator iter = m_VTKImageComponentsMap.find( image );
	if( iter != m_VTKImageComponentsMap.end() ) {
		iter->second.setVTKImageData( VolumeHandler::getVTKImageData( image, image->getImageProperties().voxelCoords[3] ) );
	}
}


void VTKImageWidgetImplementation::addImage ( const boost::shared_ptr< ImageHolder > image )
{
	VTKImageComponents component = m_VTKImageComponentsMap[image];
	m_Renderer->AddVolume( component.volume );
	component.setVTKImageData( VolumeHandler::getVTKImageData( image, image->getImageProperties().voxelCoords[3] ) );

	if( getWidgetEnsemble()->getImageVector().size() == 1 ) {
		m_Renderer->GetActiveCamera()->SetPosition( image->getImageProperties().indexOrigin[0] * 2, image->getImageProperties().indexOrigin[1] * 2, image->getImageProperties().indexOrigin[2] );
		m_Renderer->GetActiveCamera()->Roll( -65 );
	}

	const util::fvector4 io = image->getImageProperties().indexOrigin;

	const int *extent = component.getVTKImageData()->GetExtent();

	m_Cursor->SetModelBounds( extent[0] + io[0], extent[1] + io[0], extent[2] + io[1], extent[3] + io[1], extent[4] + io[2], extent[5] + io[2] );

	m_Renderer->ResetCamera();

	lookAtPhysicalCoords( image->getImageProperties().physicalCoords );

}


void VTKImageWidgetImplementation::lookAtPhysicalCoords ( const util::fvector4 &physicalCoords )
{
	if( m_ViewerCore->hasImage() ) {
		boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
		const util::ivector4 voxelCoords = image->getISISImage()->getIndexFromPhysicalCoords( physicalCoords );
		const util::fvector4 mappedVoxels = image->getImageProperties().orientation.dot( voxelCoords );
		const util::fvector4 mappedVoxelSize = image->getImageProperties().orientation.dot( image->getImageProperties().voxelSize );

		m_Cursor->SetFocalPoint( mappedVoxels[0] * fabs( mappedVoxelSize[0] ),
								 mappedVoxels[1] * fabs( mappedVoxelSize[1] ),
								 mappedVoxels[2] * fabs( mappedVoxelSize[2] ) );
		m_RenderWindow->Render();
	}
}

bool VTKImageWidgetImplementation::removeImage ( const boost::shared_ptr< ImageHolder > /*image*/ )
{
	return true;
}
void VTKImageWidgetImplementation::setInterpolationType ( InterpolationType interpolation )
{
	BOOST_FOREACH( ComponentsMapType::reference component, m_VTKImageComponentsMap ) {
		switch( interpolation ) {
		case nn:
			component.second.property->SetInterpolationTypeToNearest();
			break;
		case lin:
			component.second.property->SetInterpolationTypeToLinear();
			break;
		}
	}
}

void VTKImageWidgetImplementation::setMouseCursorIcon ( QIcon )
{
}

void VTKImageWidgetImplementation::showAnterior()
{
	const util::FixedVector<double,3> center = getCenterOfBoundingBox();
	m_Renderer->ResetCamera();
	m_Renderer->GetActiveCamera()->SetPosition(center[0],-600,center[2]);
	m_Renderer->GetActiveCamera()->SetParallelProjection(1);
	m_Renderer->GetActiveCamera()->SetViewUp(0,0,1);
	m_RenderWindow->Render();
	
}

void VTKImageWidgetImplementation::showInferior()
{
	const util::FixedVector<double,3> center = getCenterOfBoundingBox();
	m_Renderer->ResetCamera();
	m_Renderer->GetActiveCamera()->SetPosition(center[0], -center[1], -600);
	m_Renderer->GetActiveCamera()->SetParallelProjection(1);
	m_Renderer->GetActiveCamera()->SetViewUp(0,-1,0);
	m_RenderWindow->Render();
}

void VTKImageWidgetImplementation::showLeft()
{
	const util::FixedVector<double,3> center = getCenterOfBoundingBox();
	m_Renderer->ResetCamera();
	m_Renderer->GetActiveCamera()->SetPosition(600, -center[1], center[2]);
	m_Renderer->GetActiveCamera()->SetParallelProjection(1);
	m_Renderer->GetActiveCamera()->SetViewUp(0,0,1);
	m_RenderWindow->Render();

}

void VTKImageWidgetImplementation::showPosterior()
{
	const util::FixedVector<double,3> center = getCenterOfBoundingBox();
	m_Renderer->ResetCamera();
	m_Renderer->GetActiveCamera()->SetPosition(center[0],800,center[2]);
	m_Renderer->GetActiveCamera()->SetParallelProjection(1);
	m_Renderer->GetActiveCamera()->SetViewUp(0,0,1);
	m_RenderWindow->Render();
}

void VTKImageWidgetImplementation::showRight()
{
	const util::FixedVector<double,3> center = getCenterOfBoundingBox();
	m_Renderer->ResetCamera();
	m_Renderer->GetActiveCamera()->SetPosition(-600, -center[1], center[2]);
	m_Renderer->GetActiveCamera()->SetParallelProjection(1);
	m_Renderer->GetActiveCamera()->SetViewUp(0,0,1);
	m_RenderWindow->Render();
}

void VTKImageWidgetImplementation::showSuperior()
{
	const util::FixedVector<double,3> center = getCenterOfBoundingBox();
	m_Renderer->ResetCamera();
	m_Renderer->GetActiveCamera()->SetPosition(center[0], -center[1], 600);
	m_Renderer->GetActiveCamera()->SetParallelProjection(1);
	m_Renderer->GetActiveCamera()->SetViewUp(0,-1,0);
	m_RenderWindow->Render();
}


isis::util::FixedVector< double, 3 > VTKImageWidgetImplementation::getCenterOfBoundingBox()
{
	util::FixedVector<double,3> ret;
	const double *bounds = m_Actor->GetBounds();
	ret[0] = (bounds[0] - bounds[1]) / 2.0;
	ret[1] = (bounds[2] - bounds[3]) / 2.0;
	ret[2] = (bounds[4] - bounds[5]) / 2.0;
	return ret;
}


}}} //end namespace


const QWidget* loadOptionWidget()
{
	return new isis::viewer::widget::OptionWidget();
}

isis::viewer::widget::WidgetInterface *loadWidget()
{
	return new isis::viewer::widget::VTKImageWidgetImplementation();
}

const isis::util::PropertyMap *getProperties()
{
	isis::util::PropertyMap *properties = new isis::util::PropertyMap();
	properties->setPropertyAs<std::string>( "widgetIdent", "vtk_rendering_widget" );
	properties->setPropertyAs<uint8_t>( "numberOfEntitiesInEnsemble", 1 );
	properties->setPropertyAs<bool>("hasOptionWidget", true);
	return properties;
}