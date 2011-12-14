#include "viewercorebase.hpp"
#include "common.hpp"

#define STR(s) _xstr_(s)
#define _xstr_(s) std::string(#s)

namespace isis
{
namespace viewer
{

ViewerCoreBase::ViewerCoreBase( )
	: m_OptionsMap( boost::shared_ptr< util::PropertyMap >( new util::PropertyMap ) ),
	  m_CurrentAnatomicalReference( boost::shared_ptr<ImageHolder>() ),
	  m_Mode( standard )
{
	util::Singletons::get<color::Color, 10>().initStandardColormaps();
	setCommonViewerOptions();
}

ImageHolder::ImageListType ViewerCoreBase::addImageList( const std::list< data::Image > imageList, const ImageHolder::ImageType &imageType )
{
	ImageHolder::ImageListType retList;

	if( !imageList.empty() ) {
		BOOST_FOREACH( std::list< data::Image >::const_reference imageRef, imageList ) {
			retList.push_back( addImage( imageRef, imageType ) );
		}
	} else {
		LOG( Runtime, info ) << "The image list passed to the core is empty!";
	}

	return retList;
}

boost::shared_ptr<ImageHolder> ViewerCoreBase::addImage( const isis::data::Image &image, const isis::viewer::ImageHolder::ImageType &imageType )
{
	boost::shared_ptr<ImageHolder> retImage = m_DataContainer.addImage( image, imageType );
	
	//setting the lutAna
	if( imageType == ImageHolder::anatomical_image ) {
		retImage->lut = getOptionMap()->getPropertyAs<std::string>("lutAna");
	} else {
		retImage->lut = getOptionMap()->getPropertyAs<std::string>("lutZMap" );
	}
	retImage->updateColorMap();
	if( imageType == ImageHolder::anatomical_image && image.getSizeAsVector()[3] == 1 ) {
		m_CurrentAnatomicalReference = retImage;
	}

	m_ImageList.push_back( retImage );

	if( getMode() == ViewerCoreBase::zmap && retImage->getImageSize()[3] > 1 ) {
		retImage->isVisible = false;
	} else if ( getMode() == ViewerCoreBase::zmap && retImage->imageType == ImageHolder::z_map ) {
		setCurrentImage( retImage );
	} else if ( getMode() == ViewerCoreBase::standard ) {
		setCurrentImage( retImage );
	}

	return retImage;
}

boost::shared_ptr< ImageHolder > ViewerCoreBase::getCurrentImage()
{
	if( m_CurrentImage.get() ) {
		return m_CurrentImage;
	} else {
		LOG( Runtime, error ) << "Trying to fetch the current image. But there is no current image at all. Should be checked before.";
		return boost::shared_ptr< ImageHolder >();
	}

}

void ViewerCoreBase::setImageList( std::list< data::Image > imgList, const ImageHolder::ImageType &imageType )
{
	if( !imgList.empty() ) {
		m_DataContainer.clear();
	}

	ViewerCoreBase::addImageList( imgList, imageType );
}


std::string ViewerCoreBase::getVersion() const
{
	return STR( _ISIS_VIEWER_VERSION_MAJOR ) + "." + STR( _ISIS_VIEWER_VERSION_MINOR ) + "." + STR( _ISIS_VIEWER_VERSION_PATCH );
}



void ViewerCoreBase::setCommonViewerOptions()
{
	m_OptionsMap->setPropertyAs<bool>("zmapGlobal", true );
	m_OptionsMap->setPropertyAs<bool>( "propagateZooming", false );
	m_OptionsMap->setPropertyAs<bool>( "showLables", false );
	m_OptionsMap->setPropertyAs<bool>( "showCrosshair", true );
	m_OptionsMap->setPropertyAs<uint16_t>( "minMaxSearchRadius", 20 );
	m_OptionsMap->setPropertyAs<bool>( "showAdvancedFileDialogOptions", false );
	m_OptionsMap->setPropertyAs<bool>( "showFavoriteFileList", false );
	m_OptionsMap->setPropertyAs<uint16_t>( "maxWidgetHeight", 200 );
	m_OptionsMap->setPropertyAs<uint16_t>( "maxWidgetWidth", 200 );
	m_OptionsMap->setPropertyAs<uint16_t>( "maxOptionWidgetHeight", 90 );
	m_OptionsMap->setPropertyAs<uint16_t>( "minOptionWidgetHeight", 90 );
	m_OptionsMap->setPropertyAs<bool>( "showStartWidget", true );
	m_OptionsMap->setPropertyAs<bool>( "showLoadingWidget", true );
	m_OptionsMap->setPropertyAs<uint16_t>( "startWidgetHeight", 500 );
	m_OptionsMap->setPropertyAs<uint16_t>( "startWidgetWidth", 400 );
	m_OptionsMap->setPropertyAs<uint16_t>( "viewerWidgetMargin", 5 );
	m_OptionsMap->setPropertyAs<uint8_t>( "numberOfThreads", 1 );
	m_OptionsMap->setPropertyAs<bool>( "ompAvailable", false );
	m_OptionsMap->setPropertyAs<uint8_t>( "maxNumberOfThreads", 1 );
	m_OptionsMap->setPropertyAs<bool>( "enableMultithreading", false );
	m_OptionsMap->setPropertyAs<bool>( "useAllAvailableThreads", false );
	m_OptionsMap->setPropertyAs<uint8_t>( "screenshotQuality", 70 );
	m_OptionsMap->setPropertyAs<uint16_t>("screenshotWidth", 700);
	m_OptionsMap->setPropertyAs<uint16_t>("screenshotHeight", 700);
	m_OptionsMap->setPropertyAs<uint16_t>("screenshotDPIX", 300);
	m_OptionsMap->setPropertyAs<uint16_t>("screenshotDPIY", 300);	
	m_OptionsMap->setPropertyAs<bool>("screenshotManualScaling", false );
	m_OptionsMap->setPropertyAs<bool>("screenshotKeepAspectRatio", true);
	m_OptionsMap->setPropertyAs<std::string>("lutAna", "standard_grey_values" );
	m_OptionsMap->setPropertyAs<std::string>("lutZMap", "standard_zmap" );
	//logging
	m_OptionsMap->setPropertyAs<uint16_t>( "logDelayTime", 6000 );
	m_OptionsMap->setPropertyAs<bool>( "showErrorMessages", true );
	m_OptionsMap->setPropertyAs<bool>( "showNoticeMessages", true );
	m_OptionsMap->setPropertyAs<bool>( "showWarningMessages", false );
	m_OptionsMap->setPropertyAs<bool>( "showInfoMessages", false );
	m_OptionsMap->setPropertyAs<bool>( "showVerboseInfoMessages", false );
}

}
} // end namespace