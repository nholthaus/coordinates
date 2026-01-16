//--------------------------------------------------------------------------------------------------
//
//	Coordinates: A compile-time c++23 coordinate conversion library based on `units`
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2016 Nic Holthaus
//
//--------------------------------------------------------------------------------------------------
//
///	@file			dtedRawFileContents.h
///	@brief			describes the raw byte contents of a DTED file
/// @details		See MIL-PRF-89020B (JFGI) for details.
//
//--------------------------------------------------------------------------------------------------

#ifndef dtedFileContents_h
#define dtedFileContents_h

//------------------------
//	INCLUDES
//------------------------

#include <cstdint>

inline namespace coordinates
{
	inline namespace topography
	{
#pragma  pack(push)
#pragma  pack(1)
		/// DTED File contents. Source = MIL-PRF-89020B
		template<unsigned int LATITUDE_LINES, unsigned int LONGITUDE_LINES>
		struct DTEDRawFileContents
		{
#pragma  pack(push)
#pragma  pack(1)
			struct UserHeaderLabel
			{
				char UHL[3];								///< Recognition Sentinel. Value = "UHL"
				char one;									///< Fixed by standard. Value = 1
				char longitudeOfOrigin[8];					///< Lower left corner. value = DDDMMSSH, where H is the hemisphere of data
				char latitudeOfOrigin[8];					///< Lower left corner. value = DDDMMSSH, where H is the hemisphere of data
				char longitudeDataInterval[4];				///< Value = SSSS, in tenths of a second.
				char latitudeDataInterval[4];				///< Value = SSSS, in tenths of a second.
				char absoluteVerticalAccuracy_m[4];			///< Value = 0000-9999 or "NA", with 90% confidence. Right justified.
				char securityCode[3];						///< S, C, U, or R, left justified.
				char uniqueReference[12];					///< Unique reference number. May be left blank.
				char numLongitudeLines[4];					///< Count of the number of	longitude(profiles) lines for a full one - degree cell.	Count is based on the Level	of DTED and the latitude zone of the cell.
				char numLatitudeLines[4];					///< Count of the number of latitude points per longitude line for a full one - degree cell. (e.g. 1201 for DTED1, 3601.for DTED2).
				char multipleAccuracy;						///< Value = 0 (single), 1 (multiple)
				char reserved[24];							///< Reserved for future use
			} userHeaderLabel;
#pragma  pack(pop)

#pragma  pack(push)
#pragma  pack(1)
			struct DataSetIdentificationRecord
			{
				char DSI[3];								///< Recognition Sentinel. Value = "DSI"
				char securityCode;							///< S, C, U, or R.
				char releaseMarking[2];						///< Security release marking. DoD use only.
				char handlingDescription[27];				///< Security handling description. Free text, or left blank.
				char reserved0[26];							///< Reserved for future use.
				char nimaSeriesDesignator[5];				///< DTED0, DTED1, or DTED2
				char uniqueReferenceNumber[15];				///< Free text or zero filled.
				char reserved1[8];							///< Reserved for future use.
				char dataEditionNumber[2];					///< 01-99.
				char matchMergeVersion[1];					///< A-Z
				char maitenanceDate[4];						///< YYMM. Zero filled until used.
				char matchMergeDate[4];						///< YYMM. Zero filled until used.
				char maitenenceCode[4];						///< 0000 or ANNN.
				char producerCode[8];						///< CCAAABBB. (Country - Free Text) (FIPS 10 - 4 Country Codes used for first 2 characters).
				char reserved2[16];							///< Reserved for future use.
				char productSpecification[9];				///< AAAAAAAAA. Alphanumeric field.
				char version[2];							///< First digit is amendment number, second is change number
				char specificationDate[4];					///< YYMM. Date of product specification.
				char verticalDatum[3];						///< MSL, E96
				char horizontalDatum[5];					///< WGS84
				char digitizingCollectionSysytem[10];		///< Free text
				char compilationDate[4];					///< YYMM.
				char reserved3[22];							///< Reserved for future use.
				char latitideOfOrigin[9];					///< DDMMSS.SH. H is the hemisphere
				char longitudeOfOrigin[10];					///< DDDMMSS.SH. H is the hemisphere
				char latitideOfSWCorner[7];					///< DDMMSSH. H is the hemisphere
				char longitudeOfSWCorner[8];				///< DDDMMSSH. H is the hemisphere
				char latitideOfNWCorner[7];					///< DDMMSSH. H is the hemisphere
				char longitudeOfNWCorner[8];				///< DDDMMSSH. H is the hemisphere
				char latitideOfNECorner[7];					///< DDMMSSH. H is the hemisphere
				char longitudeOfNECorner[8];				///< DDDMMSSH. H is the hemisphere
				char latitideOfSECorner[7];					///< DDMMSSH. H is the hemisphere
				char longitudeOfSECorner[8];				///< DDDMMSSH. H is the hemisphere
				char orientation[9];						///< DDDMMSS.S. Clockwise orientation angle of the data with respect to true north. Usually all zeros.
				char latitudeInterval[4];					///< SSSS. Latitude interval in tenths of seconds between rows of elevation values.
				char longitudeInterval[4];					///< SSSS. Longitude interval in tenths of seconds between columns of elevation values.
				char numLatitudeLines[4];					///< 0000-9999. Number of Latitude lines. For magnetic tape, this is the actual count of the number of latitude points (rows that contain data). For CD-ROM, this is the count of the number of latitude points in a full one-degree cell. (e.g. 1201 for DTED1, 3601 for DTED2.
				char numLongitudeLines[4];					///< 0000-9999. Number of Longitude lines. For magnetic tape, this is the actual count of the number of longitude points (columns that contain data). For CD-ROM, this is the count of the number of longitude points in a full one-degree cell. The count is based on the level of DTED and the latitude zone of the cell. (See Table II and III).
				char partialCellIndicator[2];				///< 00 or 01-99. 00 = full cell coverage. 1-99 = percent coverage
				char reservedNIMA0[101];					///< Reserved for NIMA use.
				char reservedProducer[100];					///< Reserved for producing nation use.
				char comments[156];							///< Free text comments.
			} dataSetIndentificationRecord;
#pragma  pack(pop)

#pragma  pack(push)
#pragma  pack(1)
			struct AccuracyDescriptionRecord
			{
				char recognitionSentinel[3];				///< ACC
				char absoluteHorizontalAccuracy_m[4];		///< 0000-9999 or NA. meters.
				char absoluteVerticalAccuracy_m[4];			///< 0000-9999 or NA. meters.
				char relativeHorizontalAccuracy_m[4];		///< 0000-9999 or NA. meters.
				char relativeVerticalAccuracy_m[4];			///< 0000-9999 or NA. meters.
				char reserved0[4];							///< Reserved for future use.
				char reservedNIMA0[1];						///< Reserved for NIMA use only.
				char reserved1[31];							///< reserved for future use.
				char multipleAccuracyOutlineFlag[2];		///< 00 or 02-09. 00 = No accuracy subregions provided. 02 - 09 = Number of accuracy subregions per 1� cell (maximum 9).
#pragma  pack(push)
#pragma  pack(1)
				struct Subregion
				{
					char absoluteHorizontalAccuracy_m[4];	///< 0000-9999 or NA. meters.
					char absoluteVerticalAccuracy_m[4];		///< 0000-9999 or NA. meters.
					char relativeHorizontalAccuracy_m[4];	///< 0000-9999 or NA. meters.
					char relativeVerticalAccuracy_m[4];		///< 0000-9999 or NA. meters.
					char numCoordinatesInSubRegion[2];		///< 03-14. Number of coordinates in accuracy sub region outline. (Maximum of 14 coordinate pairs.The first coordinate is the most southwestern. Coordinates are input clockwise.Implied closing from last to first coordinate pairs.)
#pragma  pack(push)
#pragma  pack(1)
					struct CoordinatePair
					{
						char latitude[9];					///< DDMMSS.SH.
						char longitude[10]; 				///< DDDMMSS.SH.
					} coordinatePair[14];
#pragma  pack(pop)
				} subregion[9];
#pragma  pack(pop)
				char reservedNIMA1[18];						///< Reserved for NIMA use only;
				char reserved2[69];							///< Reserved for future use.
			} accuracyDescriptionRecord;
#pragma  pack(pop)

#pragma  pack(push)
#pragma  pack(1)
			struct DataRecord								///< Elevations are South to North within the record, and the records are West to East within the file
			{
				char sentinel;								///< 252(octal)
				char dataBlockCount[3];						///< Sequential count of the block within the file, starting with zero for the first block(Fixed Binary).
				char longitudeCount[2];						///< Count of the meridian. True longitude = longitude count x data interval + origin(Offset from the SW corner longitude) (Fixed Binary).
				char latitudeCount[2];						///< Count of the parallel. True latitude = latitude count x data interval + origin(Offset from the SW corner latitude) (Fixed Binary).
				int16_t elevation[LATITUDE_LINES];			///< True elevation value of point N of meridian in meters(Fixed Binary).
				char checksum[4];							///< Algebraic addition of contents of block.Sum is computed as an integer summation of 8 - bit values (Fixed Binary).
			} dataRecord[LONGITUDE_LINES];
#pragma  pack(pop)
		};
#pragma  pack(pop)
	}
}

#endif // dtedFileContents_h