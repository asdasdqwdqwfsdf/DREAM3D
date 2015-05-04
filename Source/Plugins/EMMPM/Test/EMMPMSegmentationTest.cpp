/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

#include <QtGui/QImage>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/DataArray.hpp"
#include "DREAM3DLib/Common/FilterPipeline.h"
#include "DREAM3DLib/Common/FilterManager.h"
#include "DREAM3DLib/Common/FilterFactory.hpp"
#include "DREAM3DLib/Plugin/IDREAM3DPlugin.h"
#include "DREAM3DLib/Plugin/DREAM3DPluginLoader.h"
#include "DREAM3DLib/Utilities/UnitTestSupport.hpp"
#include "DREAM3DLib/Utilities/QMetaObjectUtilities.h"

#include "EMMPMTestFileLocations.h"

enum ErrorCodes
{
  NO_ERROR = 0,
  COMPONENTS_DONT_MATCH = -503
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveTestFiles()
{
#if REMOVE_TEST_FILES
  QFile::remove(UnitTest::EMMPMSegmentationTest::TestFile);
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TestFilterAvailability()
{
  {
    // Now instantiate the EMMPM Filter from the FilterManager
    QString filtName = "EMMPMFilter";
    FilterManager* fm = FilterManager::Instance();
    IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);
    if (NULL == filterFactory.get())
    {
      std::stringstream ss;
      ss << "Unable to initialize the EMMPMFilter while executing the EMMPMSegmentationTest.";
      DREAM3D_TEST_THROW_EXCEPTION(ss.str())
    }
  }

  {
    // Now instantiate the MultiEmmpm Filter from the FilterManager
    QString filtName = "MultiEmmpmFilter";
    FilterManager* fm = FilterManager::Instance();
    IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);
    if (NULL == filterFactory.get())
    {
      std::stringstream ss;
      ss << "Unable to initialize the MultiEmmpmFilter while executing the EMMPMSegmentationTest.";
      DREAM3D_TEST_THROW_EXCEPTION(ss.str())
    }
  }

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void createAndAddReadImageFilter(FilterPipeline::Pointer pipeline, QString inputFile)
{
  QString filtName = "ReadImage";
  FilterManager* fm = FilterManager::Instance();
  IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);

  if (NULL != filterFactory.get())
  {
    // If we get this far, the Factory is good so creating the filter should not fail unless something has
    // horribly gone wrong in which case the system is going to come down quickly after this.
    AbstractFilter::Pointer filter = filterFactory->create();

    QVariant var;
    bool propWasSet;

    var.setValue(inputFile);
    propWasSet = filter->setProperty("InputFileName", var);
    DREAM3D_REQUIRE_EQUAL(propWasSet, true)

      pipeline->pushBack(filter);
  }
  else
  {
    QString ss = QObject::tr("EMMPMSegmentationTest Error creating filter '%1'. Filter was not created/executed. Please notify the developers.").arg(filtName);
    DREAM3D_REQUIRE_EQUAL(0, 1)
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void createAndAddConvertRGBToGrayscaleFilter(FilterPipeline::Pointer pipeline, DataArrayPath path, QString name)
{
  QString filtName = "RGBToGray";
  FilterManager* fm = FilterManager::Instance();
  IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);

  if (NULL != filterFactory.get())
  {
    // If we get this far, the Factory is good so creating the filter should not fail unless something has
    // horribly gone wrong in which case the system is going to come down quickly after this.
    AbstractFilter::Pointer filter = filterFactory->create();

    QVariant var;
    bool propWasSet;

    var.setValue(path);
    propWasSet = filter->setProperty("SelectedCellArrayArrayPath", var);
    DREAM3D_REQUIRE_EQUAL(propWasSet, true)

      var.setValue(name);
    propWasSet = filter->setProperty("NewCellArrayName", var);
    DREAM3D_REQUIRE_EQUAL(propWasSet, true)

      pipeline->pushBack(filter);
  }
  else
  {
    QString ss = QObject::tr("EMMPMSegmentationTest Error creating filter '%1'. Filter was not created/executed. Please notify the developers.").arg(filtName);
    DREAM3D_REQUIRE_EQUAL(0, 1)
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void createAndAddEMMPMFilter(FilterPipeline::Pointer pipeline, DataArrayPath input, DataArrayPath output)
{
  QString filtName = "EMMPMFilter";
  FilterManager* fm = FilterManager::Instance();
  IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);

  if (NULL != filterFactory.get())
  {
    // If we get this far, the Factory is good so creating the filter should not fail unless something has
    // horribly gone wrong in which case the system is going to come down quickly after this.
    AbstractFilter::Pointer filter = filterFactory->create();

    QVariant var;
    int err = 0;
    bool propWasSet;

    var.setValue(input);
    propWasSet = filter->setProperty("InputDataArrayPath", var);
    DREAM3D_REQUIRE_EQUAL(propWasSet, true)

      var.setValue(output);
    propWasSet = filter->setProperty("OutputDataArrayPath", var);
    DREAM3D_REQUIRE_EQUAL(propWasSet, true)

      pipeline->pushBack(filter);
  }
  else
  {
    QString ss = QObject::tr("EMMPMSegmentationTest Error creating filter '%1'. Filter was not created/executed. Please notify the developers.").arg(filtName);
    DREAM3D_REQUIRE_EQUAL(0, 1)
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void createAndAddMultiEMMPMFilter(FilterPipeline::Pointer pipeline, QVector<DataArrayPath> input)
{
  QString filtName = "MultiEmmpmFilter";
  FilterManager* fm = FilterManager::Instance();
  IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);

  if (NULL != filterFactory.get())
  {
    // If we get this far, the Factory is good so creating the filter should not fail unless something has
    // horribly gone wrong in which case the system is going to come down quickly after this.
    AbstractFilter::Pointer filter = filterFactory->create();

    QVariant var;
    bool propWasSet;

    var.setValue(input);
    propWasSet = filter->setProperty("InputDataArrayVector", var);
    DREAM3D_REQUIRE_EQUAL(propWasSet, true)

      pipeline->pushBack(filter);
  }
  else
  {
    QString ss = QObject::tr("EMMPMSegmentationTest Error creating filter '%1'. Filter was not created/executed. Please notify the developers.").arg(filtName);
    DREAM3D_REQUIRE_EQUAL(0, 1)
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TestEMMPMSegmentation()
{
  QImage inputImage(100, 100, QImage::Format_ARGB32);
  bool saveResult = inputImage.save(UnitTest::EMMPMSegmentationTest::TestFile);
  DREAM3D_REQUIRE_EQUAL(saveResult, true)

  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  {

    createAndAddReadImageFilter(pipeline, UnitTest::EMMPMSegmentationTest::TestFile);
    createAndAddConvertRGBToGrayscaleFilter(pipeline, DataArrayPath("DataContainer", "CellData", "ImageData"), "Gray");
    createAndAddEMMPMFilter(pipeline, DataArrayPath("DataContainer", "CellData", "Gray"), DataArrayPath("DataContainer", "CellData", "Test"));

    pipeline->execute();
    DREAM3D_REQUIRE_EQUAL(pipeline->getErrorCondition(), NO_ERROR)
  }

  pipeline->clear();

  {
    createAndAddReadImageFilter(pipeline, UnitTest::EMMPMSegmentationTest::TestFile);
    //createAndAddConvertRGBToGrayscaleFilter(pipeline, DataArrayPath("DataContainer", "CellData", "ImageData"), "Gray");
    createAndAddEMMPMFilter(pipeline, DataArrayPath("DataContainer", "CellData", "ImageData"), DataArrayPath("DataContainer", "CellData", "Test"));

    pipeline->execute();
    DREAM3D_REQUIRE_EQUAL(pipeline->getErrorCondition(), COMPONENTS_DONT_MATCH)
  }

    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TestMultiEMMPMSegmentation()
{
  QImage inputImage(100, 100, QImage::Format_ARGB32);
  bool saveResult = inputImage.save(UnitTest::EMMPMSegmentationTest::TestFile);
  DREAM3D_REQUIRE_EQUAL(saveResult, true)

  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  {
    createAndAddReadImageFilter(pipeline, UnitTest::EMMPMSegmentationTest::TestFile);
    createAndAddConvertRGBToGrayscaleFilter(pipeline, DataArrayPath("DataContainer", "CellData", "ImageData"), "Gray");

    QVector<DataArrayPath> vector;
    vector.push_back(DataArrayPath("DataContainer", "CellData", "Gray"));
    createAndAddMultiEMMPMFilter(pipeline, vector);
    pipeline->execute();
    DREAM3D_REQUIRE_EQUAL(pipeline->getErrorCondition(), NO_ERROR)
  }

  pipeline->clear();

  {
    createAndAddReadImageFilter(pipeline, UnitTest::EMMPMSegmentationTest::TestFile);
    createAndAddConvertRGBToGrayscaleFilter(pipeline, DataArrayPath("DataContainer", "CellData", "ImageData"), "Gray");

    QVector<DataArrayPath> vector;
    vector.push_back(DataArrayPath("DataContainer", "CellData", "Gray"));
    vector.push_back(DataArrayPath("DataContainer", "CellData", "ImageData"));
    createAndAddMultiEMMPMFilter(pipeline, vector);
    pipeline->execute();
    DREAM3D_REQUIRE_EQUAL(pipeline->getErrorCondition(), COMPONENTS_DONT_MATCH)
  }

  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loadFilterPlugins()
{
  // Register all the filters including trying to load those from Plugins
  FilterManager* fm = FilterManager::Instance();
  DREAM3DPluginLoader::LoadPluginFilters(fm);

  // Send progress messages from PipelineBuilder to this object for display
  QMetaObjectUtilities::RegisterMetaTypes();
}


// -----------------------------------------------------------------------------
//  Use test framework
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // Instantiate the QCoreApplication that we need to get the current path and load plugins.
  QCoreApplication app(argc, argv);
  QCoreApplication::setOrganizationName("BlueQuartz Software");
  QCoreApplication::setOrganizationDomain("bluequartz.net");
  QCoreApplication::setApplicationName("EMMPMSegmentationTest");

  int err = EXIT_SUCCESS;
  DREAM3D_REGISTER_TEST( loadFilterPlugins() );
  DREAM3D_REGISTER_TEST( TestFilterAvailability() );

  DREAM3D_REGISTER_TEST(TestEMMPMSegmentation())
  DREAM3D_REGISTER_TEST(TestMultiEMMPMSegmentation())

  DREAM3D_REGISTER_TEST(RemoveTestFiles())
  PRINT_TEST_SUMMARY();

  return err;
}
