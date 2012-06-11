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
 * Author: Erik Tuerke, tuerke@cbs.mpg.de
 *
 * memoryhandler.hpp
 *
 * Description:
 *
 *  Created on: Mar 16, 2012
 *      Author: tuerke
 ******************************************************************/
#ifndef VAST_MEMORY_HANDLER_HPP
#define VAST_MEMORY_HANDLER_HPP

#include <CoreUtils/vector.hpp>
#include <DataStorage/chunk.hpp>
#include <common.hpp>

namespace isis
{
namespace viewer
{

class MemoryHandler
{
public:
	static util::ivector4 get32BitAlignedSize( const util::ivector4 &origSize );

	template< typename TYPE>
	static void fillSliceChunk( data::MemChunk<TYPE> &sliceChunk, const ImageHolder::Pointer image, const PlaneOrientation &orientation ) {
		const util::ivector4 mappedSize = mapCoordsToOrientation( image->getImageSize(), image->getImageProperties().latchedOrientation, orientation );
		const util::ivector4 mappedCoords = mapCoordsToOrientation( image->getImageProperties().trueVoxelCoords, image->getImageProperties().latchedOrientation, orientation );
		const util::ivector4 mapping = mapCoordsToOrientation( util::ivector4( 0, 1, 2, 3 ), image->getImageProperties().latchedOrientation, orientation, true );
		const data::Chunk &chunk = image->getVolumeVector()[image->getImageProperties().voxelCoords[dim_time]];
		
		const bool sliceIsInside = image->getImageProperties().trueVoxelCoords[mapping[2]] >= 0 && image->getImageProperties().trueVoxelCoords[mapping[2]] < mappedSize[2];
		
		for ( util::ivector4::value_type y = 0; y < mappedSize[1]; y++ ) {
			for ( util::ivector4::value_type x = 0; x < mappedSize[0]; x++ ) {
				const util::ivector4::value_type coords[3] = {x, y, mappedCoords[2] };
				if( sliceIsInside ) {
					static_cast<data::Chunk &>( sliceChunk ).voxel<TYPE>( x, y ) = chunk.voxel<TYPE>( coords[mapping[0]], coords[mapping[1]], coords[mapping[2]] );
				}
			}
		}
	}

	template< typename TYPE>
	static void fillSliceChunkOriented( data::MemChunk<TYPE> &sliceChunk, const ImageHolder::Pointer image, const PlaneOrientation &orientation ) {
		if( image->getImageProperties().latchedOrientation == image->getImageProperties().orientation ) {
			fillSliceChunk<TYPE>( sliceChunk, image, orientation );
		} else {
			const data::Chunk &chunk = image->getVolumeVector()[image->getImageProperties().voxelCoords[dim_time]];
			boost::shared_ptr< data::Image > isisImage = image->getISISImage();
			const geometrical::BoundingBoxType &bb = image->getImageProperties().boundingBox;
			const util::ivector4 mapping = mapCoordsToOrientation( util::fvector4( 0, 1, 2 ), image->getImageProperties().latchedOrientation, orientation );
			const util::ivector4 _mapping = mapCoordsToOrientation( util::fvector4( 0, 1, 2 ), util::IdentityMatrix<float, 4>(), orientation );
			const util::fvector4 vS = image->getImageProperties().orientation.dot( image->getImageProperties().voxelSize );
			util::fvector4 phys = image->getImageProperties().physicalCoords;
			float factor = 1. / sqrt( 2 );
			const float stepI = factor * image->getImageProperties().voxelSize[mapping[0]];
			const float stepJ = factor * image->getImageProperties().voxelSize[mapping[1]];

			for ( float j = bb[_mapping[1]].first; j < bb[_mapping[1]].second; j += stepJ  ) {
				phys[_mapping[1]] = j;

				for ( float i = bb[_mapping[0]].first; i < bb[_mapping[0]].second; i += stepI ) {
					phys[_mapping[0]] = i;
					const util::ivector4 voxCoords = isisImage->getIndexFromPhysicalCoords( phys, false );

					if( image->checkVoxelCoords( voxCoords ) ) {
						static_cast<data::Chunk & >( sliceChunk ).voxel<TYPE>( voxCoords[mapping[0]], voxCoords[mapping[1]] ) =
							chunk.voxel<TYPE>( voxCoords[0], voxCoords[1], voxCoords[2] );
					}
				}
			}

		}
	}


};



}
} //end namespace



#endif //VAST_MEMORY_HANDLER_HPP