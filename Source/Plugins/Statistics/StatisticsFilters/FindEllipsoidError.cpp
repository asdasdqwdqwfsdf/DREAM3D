/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "FindEllipsoidError.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "DREAM3DLib/Math/DREAM3DMath.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

FindEllipsoidError::FindEllipsoidError()  :
  AbstractFilter(),
  m_CellFeatureAttributeMatrixName(DREAM3D::Defaults::DataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, ""),
  m_FeatureIdsArrayPath(DREAM3D::Defaults::DataContainerName, DREAM3D::Defaults::CellAttributeMatrixName, DREAM3D::CellData::FeatureIds),
  m_CentroidsArrayPath(DREAM3D::Defaults::DataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::Centroids),
  m_NumCellsArrayPath(DREAM3D::Defaults::DataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::NumCells),
  m_AxisLengthsArrayPath(DREAM3D::Defaults::DataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::AxisLengths),
  m_AxisEulerAnglesArrayPath(DREAM3D::Defaults::DataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::AxisEulerAngles),
  m_FeatureIdsArrayName(DREAM3D::CellData::FeatureIds),
  m_IdealFeatureIdsArrayName("IdealFeatureIds"),
  m_EllipsoidErrorArrayName("EllipsoidError"),
  m_FeatureIds(NULL),
  m_AxisEulerAngles(NULL),
  m_CentroidsArrayName(DREAM3D::FeatureData::Centroids),
  m_Centroids(NULL),
  m_AxisLengths(NULL),
  m_NumCells(NULL),
  m_IdealFeatureIds(NULL),
  m_EllipsoidError(NULL),
  m_WriteIdealEllipseFeatureIds(true)


{


  scaleFactor = 1.0;

  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindEllipsoidError::~FindEllipsoidError()
{
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEllipsoidError::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FilterParameter::New("Required Information", "", FilterParameterWidgetType::SeparatorWidget, "", true));
  parameters.push_back(FilterParameter::New("FeatureIds", "FeatureIdsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getFeatureIdsArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("Centroids", "CentroidsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getCentroidsArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("NumCells", "NumCellsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getNumCellsArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("Axis Eulers", "AxisEulerAnglesArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getAxisEulerAnglesArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("AxisLengths", "AxisLengthsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getAxisLengthsArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("Cell Feature Attribute Matrix Name", "CellFeatureAttributeMatrixName", FilterParameterWidgetType::AttributeMatrixSelectionWidget, getCellFeatureAttributeMatrixName(), true, ""));
  parameters.push_back(FilterParameter::New("Created Information", "", FilterParameterWidgetType::SeparatorWidget, "", true));
  parameters.push_back(FilterParameter::New("EllipsoidError", "EllipsoidErrorArrayName", FilterParameterWidgetType::StringWidget, getEllipsoidErrorArrayName(), true, ""));
  QStringList linkedProps("IdealFeatureIdsArrayName");
  parameters.push_back(LinkedBooleanFilterParameter::New("Write Ideal Ellipse Feature Ids (Caution LONG calculation)", "WriteIdealEllipseFeatureIds", getWriteIdealEllipseFeatureIds(), linkedProps, false));
  parameters.push_back(FilterParameter::New("IdealFeatureIds", "IdealFeatureIdsArrayName", FilterParameterWidgetType::StringWidget, getIdealFeatureIdsArrayName(), false, ""));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEllipsoidError::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setCellFeatureAttributeMatrixName(reader->readDataArrayPath("CellFeatureAttributeMatrixName", getCellFeatureAttributeMatrixName()));
  setCentroidsArrayPath(reader->readDataArrayPath("CentroidsArrayPath", getCentroidsArrayPath() ) );
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath() ) );
  setAxisEulerAnglesArrayPath(reader->readDataArrayPath("AxisEulerAnglesArrayPath", getAxisEulerAnglesArrayPath()));
  setNumCellsArrayPath(reader->readDataArrayPath("NumCellsArrayPath", getNumCellsArrayPath()));
  setAxisLengthsArrayPath(reader->readDataArrayPath("AxisLengthsArrayPath", getAxisLengthsArrayPath()));
  setIdealFeatureIdsArrayName(reader->readString("IdealFeatureIdsArrayName", getIdealFeatureIdsArrayName() ) );
  setEllipsoidErrorArrayName(reader->readString("EllipsoidErrrorArrayName", getEllipsoidErrorArrayName() ) );
  setWriteIdealEllipseFeatureIds(reader->readValue("WriteIdealEllipseFeatureIds", getWriteIdealEllipseFeatureIds() ) );

  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int FindEllipsoidError::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(CellFeatureAttributeMatrixName)
  DREAM3D_FILTER_WRITE_PARAMETER(AxisEulerAnglesArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(AxisLengthsArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(NumCellsArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(CentroidsArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(FeatureIdsArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(IdealFeatureIdsArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(EllipsoidErrorArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(WriteIdealEllipseFeatureIds)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEllipsoidError::dataCheck()
{
  DataArrayPath tempPath;
  setErrorCondition(0);


  QVector<size_t> dims(1, 1);
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this, getFeatureIdsArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_FeatureIdsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCondition() < 0) { return; }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getFeatureIdsArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || NULL == image.get()) { return; }

  dims[0] = 3;
  m_CentroidsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>, AbstractFilter>(this, getCentroidsArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_CentroidsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_Centroids = m_CentroidsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  dims[0] = 3;
  m_AxisEulerAnglesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>, AbstractFilter>(this, getAxisEulerAnglesArrayPath(), dims);
  if( NULL != m_AxisEulerAnglesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_AxisEulerAngles = m_AxisEulerAnglesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  dims[0] = 3;
  m_AxisLengthsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<float>, AbstractFilter>(this, getAxisLengthsArrayPath(), dims);
  if( NULL != m_AxisLengthsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_AxisLengths = m_AxisLengthsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  dims[0] = 1;
  m_NumCellsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this, getNumCellsArrayPath(), dims);
  if( NULL != m_NumCellsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_NumCells = m_NumCellsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  if (m_WriteIdealEllipseFeatureIds == true)
  {
      dims[0] = 1;
      tempPath.update(m_FeatureIdsArrayPath.getDataContainerName(), m_FeatureIdsArrayPath.getAttributeMatrixName(), getIdealFeatureIdsArrayName() );
      m_IdealFeatureIdsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter, int32_t>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
      if( NULL != m_IdealFeatureIdsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
      { m_IdealFeatureIds = m_IdealFeatureIdsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  }

  dims[0] = 1;
  tempPath.update(m_NumCellsArrayPath.getDataContainerName(), m_NumCellsArrayPath.getAttributeMatrixName(), getEllipsoidErrorArrayName() );
  m_EllipsoidErrorPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>, AbstractFilter, float>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_EllipsoidErrorPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_EllipsoidError = m_EllipsoidErrorPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEllipsoidError::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEllipsoidError::execute()
{
  setErrorCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName());
  float xRes = m->getGeometryAs<ImageGeom>()->getXRes();
  float yRes = m->getGeometryAs<ImageGeom>()->getYRes();
  float zRes = m->getGeometryAs<ImageGeom>()->getZRes();



  scaleFactor = 1.0 / xRes;
  if(yRes > xRes && yRes > zRes) { scaleFactor = 1.0 / yRes; }
  if(zRes > xRes && zRes > yRes) { scaleFactor = 1.0 / zRes; }

  if(m->getGeometryAs<ImageGeom>()->getXPoints() > 1 && m->getGeometryAs<ImageGeom>()->getYPoints() > 1 && m->getGeometryAs<ImageGeom>()->getZPoints() > 1) {  }
  if(m->getGeometryAs<ImageGeom>()->getXPoints() == 1 || m->getGeometryAs<ImageGeom>()->getYPoints() == 1 || m->getGeometryAs<ImageGeom>()->getZPoints() == 1) {find_error2D();}


  notifyStatusMessage(getHumanLabel(), "FindEllipsoidError Completed");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindEllipsoidError::find_error2D()
{
    size_t numfeatures = m_NumCellsPtr.lock()->getNumberOfTuples();
    std::vector<std::vector<size_t> > featureCellList(numfeatures);
    std::vector<size_t> numInsideIdeal(numfeatures, 0);  //Create a list for the ideal ellipses
    std::vector<size_t> numOutsideIdeal(numfeatures, 0);

    DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName());
    size_t xPoints = m->getGeometryAs<ImageGeom>()->getXPoints();
    size_t yPoints = m->getGeometryAs<ImageGeom>()->getYPoints();
    size_t zPoints = m->getGeometryAs<ImageGeom>()->getZPoints();

    float xRes = m->getGeometryAs<ImageGeom>()->getXRes();
    float yRes = m->getGeometryAs<ImageGeom>()->getYRes();
    //float zRes = m->getGeometryAs<ImageGeom>()->getZRes();

    float xsquared, ysquared, asquared, bsquared, xc, yc, theta;
    int32_t xcoord, ycoord;

    size_t totalPoints = xPoints * yPoints * zPoints;

    for(size_t k=0; k < totalPoints; k++)
    {
        featureCellList[m_FeatureIds[k]].push_back(k);


//        //calculate the x and y coordinate for each cell in the actual feature
//        xcoord = int(k % xPoints);
//        ycoord = int(k/xPoints) % yPoints;
//        for (size_t i = 1; i<numfeatures; i++)
//        {
//            theta = -m_AxisEulerAngles[3*i]; //only need the first angle in 2D

//            //Get the centroids (in pixels) for the ideal ellipse
//            xc = m_Centroids[3*i]/xRes;
//            yc = m_Centroids[3*i + 1]/yRes;

//            //Get the axis lengths for the ideal ellipse
//            asquared = (m_AxisLengths[3*i]*m_AxisLengths[3*i])/(xRes*xRes);
//            bsquared = m_AxisLengths[3*i+1]*m_AxisLengths[3*i+1]/(yRes*yRes);

//            //rotate and translate the current x, y pair into where the ideal ellipse is
//            xsquared = ((xcoord-xc)*cosf(theta)-(ycoord-yc)*sinf(theta))*((xcoord-xc)*cosf(theta)-(ycoord-yc)*sinf(theta));
//            ysquared = ((xcoord-xc)*sinf(theta)+(ycoord-yc)*cosf(theta))*((xcoord-xc)*sinf(theta)+(ycoord-yc)*cosf(theta));

//            if( (xsquared/asquared + ysquared/bsquared) < 1 )
//            {
//                m_IdealFeatureIds[k] = i;
//            }

//        }
    }


    for (size_t i=1; i<numfeatures; i++)
    {
        theta = -m_AxisEulerAngles[3*i]; //only need the first angle in 2D

        //Get the centroids (in pixels) for the ideal ellipse
        xc = m_Centroids[3*i]/xRes;
        yc = m_Centroids[3*i + 1]/yRes;

        //Get the axis lengths for the ideal ellipse
        asquared = (m_AxisLengths[3*i]*m_AxisLengths[3*i])/(xRes*xRes);
        bsquared = m_AxisLengths[3*i+1]*m_AxisLengths[3*i+1]/(yRes*yRes);

        //iterate over all the cells in each feature
        for(size_t j=0; j<featureCellList[i].size(); j++)
        {
            //calculate the x and y coordinate for each cell in the actual feature
            xcoord = int(featureCellList[i][j] % xPoints);
            ycoord = int(featureCellList[i][j]/xPoints) % yPoints;

            //rotate and translate the current x, y pair into where the ideal ellipse is
            xsquared = ((xcoord-xc)*cosf(theta)-(ycoord-yc)*sinf(theta))*((xcoord-xc)*cosf(theta)-(ycoord-yc)*sinf(theta));
            ysquared = ((xcoord-xc)*sinf(theta)+(ycoord-yc)*cosf(theta))*((xcoord-xc)*sinf(theta)+(ycoord-yc)*cosf(theta));

            if( (xsquared/asquared + ysquared/bsquared) < 1 )
            {
                numInsideIdeal[i]++;
            }
            else
            {
                numOutsideIdeal[i]++;
            }

        }

        m_EllipsoidError[i] = float(numOutsideIdeal[i])/float(m_NumCells[i]);

    }






    std::cout << "done" << std::endl;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------







// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindEllipsoidError::newFilterInstance(bool copyFilterParameters)
{
  FindEllipsoidError::Pointer filter = FindEllipsoidError::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindEllipsoidError::getCompiledLibraryName()
{ return StatisticsConstants::StatisticsBaseName; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindEllipsoidError::getGroupName()
{ return DREAM3D::FilterGroups::StatisticsFilters; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindEllipsoidError::getSubGroupName()
{ return DREAM3D::FilterSubGroups::MorphologicalFilters; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindEllipsoidError::getHumanLabel()
{ return "Find Ellipsoid Error"; }
