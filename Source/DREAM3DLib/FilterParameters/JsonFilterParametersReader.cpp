/* ============================================================================
* Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
* Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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
*                           FA8650-10-D-5210
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "JsonFilterParametersReader.h"

#include <QtCore/QMetaType>
#include <QtCore/QDataStream>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>

#include "DREAM3DLib/Common/FilterManager.h"
#include "DREAM3DLib/Common/FilterFactory.hpp"
#include "DREAM3DLib/CoreFilters/EmptyFilter.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
JsonFilterParametersReader::JsonFilterParametersReader() :
AbstractFilterParametersReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
JsonFilterParametersReader::~JsonFilterParametersReader()
{
  
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer JsonFilterParametersReader::ReadPipelineFromFile(QString filePath, IObserver* obs)
{
  if (filePath.isEmpty() == true)
  {
    return FilterPipeline::NullPointer();
  }
  QFileInfo fi(filePath);
  if (fi.exists() == false)
  {
    return FilterPipeline::NullPointer();
  }

  FilterManager* filtManager = FilterManager::Instance();
  FilterFactory<EmptyFilter>::Pointer emptyFilterFactory = FilterFactory<EmptyFilter>::New();
  filtManager->addFilterFactory("EmptyFilter", emptyFilterFactory);

  JsonFilterParametersReader::Pointer reader = JsonFilterParametersReader::New();
  reader->openFile(filePath);
  reader->openGroup(DREAM3D::Settings::PipelineBuilderGroup);
  bool ok = false;
  int filterCount = reader->readValue(DREAM3D::Settings::NumFilters, 0);
  reader->closeGroup();

  FilterPipeline::Pointer pipeline = FilterPipeline::New();


  if (false == ok)
  {
    filterCount = 0;
  }

  for (int i = 0; i < filterCount; ++i)
  {
    // Open the group to get the name of the filter then close again.
    //qDebug() << " Current QSettings Group: " << prefs->group();
    reader->openFilterGroup(NULL, i);
    QString filterName = reader->readString(DREAM3D::Settings::FilterName, "");
    reader->closeFilterGroup();
    //qDebug() << "Group: " << gName << " FilterName: " << filterName;
    if (filterName.isEmpty() == false)
    {
      IFilterFactory::Pointer factory = filtManager->getFactoryForFilter(filterName);
      if (factory.get() != NULL)
      {
        AbstractFilter::Pointer filter = factory->create();

        if (NULL != filter.get())
        {
          filter->readFilterParameters(reader.get(), i);
          pipeline->pushBack(filter);
        }
      }
      else // Could not find the filter because the specific name has not been registered. This could
        // be due to a name change for the filter.
      {
        EmptyFilter::Pointer filter = EmptyFilter::New();
        QString humanLabel = QString("UNKNOWN FILTER: ") + filterName;
        filter->setHumanLabel(humanLabel);
        filter->setOriginalFilterName(filterName);
        pipeline->pushBack(filter);

        if (NULL != obs)
        {
          QString ss = QObject::tr("An implementation for filter '%1' could not be located. Possible reasons include a name change of the filter, plugin not loading or a simple spelling mistake? A blank filter has been inserted in its place.").arg(filterName);
          PipelineMessage pm(filterName, ss, -66066, PipelineMessage::Error);
          pm.setPrefix("JsonFilterParametersReader::ReadPipelineFromFile()");
          obs->processPipelineMessage(pm);
        }
      }
    }
    else
    {
      EmptyFilter::Pointer filter = EmptyFilter::New();
      QString humanLabel = QString("MISSING FILTER: ") + filterName;
      filter->setHumanLabel(humanLabel);
      filter->setOriginalFilterName(filterName);
      pipeline->pushBack(filter);

      if (NULL != obs)
      {
        QString gName = QString::number(i);
        QString ss = QObject::tr("A filter for index '%1' is missing in the file. Is the numbering of the filters correct in the pipeline file?").arg(gName);
        PipelineMessage pm(filterName, ss, -66067, PipelineMessage::Error);
        pm.setPrefix("JsonFilterParametersReader::ReadPipelineFromFile()");
        obs->processPipelineMessage(pm);
      }
    }


  }
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int JsonFilterParametersReader::openFile(QString filePath)
{
  if (m_Root.isEmpty() == false || m_CurrentFilterIndex.isEmpty() == false)
  {
    closeFile();
  }

  QFile inputFile(filePath);
  if (inputFile.open(QIODevice::ReadOnly))
  {
    QJsonParseError parseError;
    QByteArray byteArray = inputFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
      return parseError.error;
    }
    m_Root = doc.object();

    return 0;
  }

  return -100;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void JsonFilterParametersReader::closeFile()
{
  m_Root = QJsonObject();
  m_CurrentFilterIndex = QJsonObject();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int JsonFilterParametersReader::openFilterGroup(AbstractFilter* unused, int index)
{
  BOOST_ASSERT(m_Root.isEmpty() == false);
  int err = 0;
  QString numStr = QString::number(index);
  m_CurrentFilterIndex = m_Root[numStr].toObject();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int JsonFilterParametersReader::closeFilterGroup()
{
  BOOST_ASSERT(m_Root.isEmpty() == false);
  m_CurrentFilterIndex = QJsonObject();
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int JsonFilterParametersReader::openGroup(QString key)
{
  BOOST_ASSERT(m_Root.isEmpty() == false);
  int err = 0;
  m_CurrentFilterIndex = m_Root[key].toObject();
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int JsonFilterParametersReader::closeGroup()
{
  BOOST_ASSERT(m_Root.isEmpty() == false);
  m_CurrentFilterIndex = QJsonObject();
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString JsonFilterParametersReader::readString(const QString name, QString value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }
  return m_CurrentFilterIndex.value(name).toString();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<QString> JsonFilterParametersReader::readStrings(const QString name, QVector<QString> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
  QVector<QString> vector;
  for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
  {
    if ((*iter).isString())
    {
      QString str = (*iter).toString();
      vector.push_back(str);
    }
  }

  return vector;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int8_t JsonFilterParametersReader::readValue(const QString name, int8_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<int8_t>().min() && val <= std::numeric_limits<int8_t>().max())
    {
      return static_cast<int8_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int16_t JsonFilterParametersReader::readValue(const QString name, int16_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<int16_t>().min() && val <= std::numeric_limits<int16_t>().max())
    {
      return static_cast<int16_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t JsonFilterParametersReader::readValue(const QString name, int32_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<int32_t>().min() && val <= std::numeric_limits<int32_t>().max())
    {
      return static_cast<int32_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int64_t JsonFilterParametersReader::readValue(const QString name, int64_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<int64_t>().min() && val <= std::numeric_limits<int64_t>().max())
    {
      return static_cast<int64_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint8_t JsonFilterParametersReader::readValue(const QString name, uint8_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<uint8_t>().min() && val <= std::numeric_limits<uint8_t>().max())
    {
      return static_cast<uint8_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint16_t JsonFilterParametersReader::readValue(const QString name, uint16_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<uint16_t>().min() && val <= std::numeric_limits<uint16_t>().max())
    {
      return static_cast<uint16_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint32_t JsonFilterParametersReader::readValue(const QString name, uint32_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<uint32_t>().min() && val <= std::numeric_limits<uint32_t>().max())
    {
      return static_cast<uint32_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
uint64_t JsonFilterParametersReader::readValue(const QString name, uint64_t value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<uint64_t>().min() && val <= std::numeric_limits<uint64_t>().max())
    {
      return static_cast<uint64_t>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
float JsonFilterParametersReader::readValue(const QString name, float value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    double val = m_CurrentFilterIndex.value(name).toDouble();
    if (val >= std::numeric_limits<float>().min() && val <= std::numeric_limits<float>().max())
    {
      return static_cast<float>(val);
    }
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double JsonFilterParametersReader::readValue(const QString name, double value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isDouble())
  {
    return m_CurrentFilterIndex.value(name).toDouble();
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool JsonFilterParametersReader::readValue(const QString name, bool value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isBool())
  {
    return m_CurrentFilterIndex.value(name).toBool();
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int8_t> JsonFilterParametersReader::readArray(const QString name, QVector<int8_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<int8_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<int8_t>().min() && val <= std::numeric_limits<int8_t>().max())
        {
          vector.push_back(static_cast<int8_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int16_t> JsonFilterParametersReader::readArray(const QString name, QVector<int16_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<int16_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<int16_t>().min() && val <= std::numeric_limits<int16_t>().max())
        {
          vector.push_back(static_cast<int16_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int32_t> JsonFilterParametersReader::readArray(const QString name, QVector<int32_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<int32_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<int32_t>().min() && val <= std::numeric_limits<int32_t>().max())
        {
          vector.push_back(static_cast<int32_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<int64_t> JsonFilterParametersReader::readArray(const QString name, QVector<int64_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<int64_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<int64_t>().min() && val <= std::numeric_limits<int64_t>().max())
        {
          vector.push_back(static_cast<int64_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint8_t> JsonFilterParametersReader::readArray(const QString name, QVector<uint8_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<uint8_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<uint8_t>().min() && val <= std::numeric_limits<uint8_t>().max())
        {
          vector.push_back(static_cast<uint8_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint16_t> JsonFilterParametersReader::readArray(const QString name, QVector<uint16_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<uint16_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<uint16_t>().min() && val <= std::numeric_limits<uint16_t>().max())
        {
          vector.push_back(static_cast<uint16_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint32_t> JsonFilterParametersReader::readArray(const QString name, QVector<uint32_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<uint32_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<uint32_t>().min() && val <= std::numeric_limits<uint32_t>().max())
        {
          vector.push_back(static_cast<uint32_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<uint64_t> JsonFilterParametersReader::readArray(const QString name, QVector<uint64_t> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<uint64_t> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<uint64_t>().min() && val <= std::numeric_limits<uint64_t>().max())
        {
          vector.push_back(static_cast<uint64_t>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<float> JsonFilterParametersReader::readArray(const QString name, QVector<float> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<float> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        double val = (*iter).toDouble();
        if (val >= std::numeric_limits<float>().min() && val <= std::numeric_limits<float>().max())
        {
          vector.push_back(static_cast<float>(val));
        }
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<double> JsonFilterParametersReader::readArray(const QString name, QVector<double> value)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return value;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex.value(name).toArray();
    QVector<double> vector;
    for (QJsonArray::iterator iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
      if ((*iter).isDouble())
      {
        vector.push_back((*iter).toDouble());
      }
    }
    return vector;
  }

  return value;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
IntVec3_t JsonFilterParametersReader::readIntVec3(const QString name, IntVec3_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    IntVec3_t vec3;
    if (vec3.readJson(jsonObject) == true)
    {
      return vec3;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatVec3_t JsonFilterParametersReader::readFloatVec3(const QString name, FloatVec3_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    FloatVec3_t vec3;
    if (vec3.readJson(jsonObject) == true)
    {
      return vec3;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatVec4_t JsonFilterParametersReader::readFloatVec4(const QString name, FloatVec4_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    FloatVec4_t vec4;
    if (vec4.readJson(jsonObject) == true)
    {
      return vec4;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatVec21_t JsonFilterParametersReader::readFloatVec21(const QString name, FloatVec21_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    FloatVec21_t vec21;
    if (vec21.readJson(jsonObject) == true)
    {
      return vec21;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Float2ndOrderPoly_t JsonFilterParametersReader::readFloat2ndOrderPoly(const QString name, Float2ndOrderPoly_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    Float2ndOrderPoly_t poly;
    if (poly.readJson(jsonObject) == true)
    {
      return poly;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Float3rdOrderPoly_t JsonFilterParametersReader::readFloat3rdOrderPoly(const QString name, Float3rdOrderPoly_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    Float3rdOrderPoly_t poly;
    if (poly.readJson(jsonObject) == true)
    {
      return poly;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Float4thOrderPoly_t JsonFilterParametersReader::readFloat4thOrderPoly(const QString name, Float4thOrderPoly_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    Float4thOrderPoly_t poly;
    if (poly.readJson(jsonObject) == true)
    {
      return poly;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FileListInfo_t JsonFilterParametersReader::readFileListInfo(const QString name, FileListInfo_t defaultValue)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonObject jsonObject = m_CurrentFilterIndex.value(name).toObject();
    FileListInfo_t fLInfo;
    if (fLInfo.readJson(jsonObject) == true)
    {
      return fLInfo;
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonInput_t JsonFilterParametersReader::readComparisonInput(const QString name, ComparisonInput_t defaultValue, int arrayIndex)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defaultValue;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray cInputsArray = m_CurrentFilterIndex.value(name).toArray();
    if (cInputsArray.size() > arrayIndex && cInputsArray[arrayIndex].isObject())
    {
      QJsonObject cInputObject = cInputsArray[arrayIndex].toObject();
      ComparisonInput_t cInput;
      if (cInput.readJson(cInputObject) == true)
      {
        return cInput;
      }
    }
  }

  return defaultValue;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ComparisonInputs JsonFilterParametersReader::readComparisonInputs(const QString name, ComparisonInputs defValues)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return defValues;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray cInputsArray = m_CurrentFilterIndex.value(name).toArray();

    ComparisonInputs inputs;
    for (int i = 0; i < cInputsArray.size(); i++)
    {
      if (cInputsArray[i].isObject() == true)
      {
        QJsonObject cInputObj = cInputsArray[i].toObject();
        ComparisonInput_t cInput;
        if (cInput.readJson(cInputObj) == true)
        {
          inputs.addInput(cInput);
        }
        else
        {
          return defValues;
        }
      }
    }
    return inputs;
  }

  return defValues;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AxisAngleInput_t JsonFilterParametersReader::readAxisAngle(const QString name, AxisAngleInput_t v, int vectorPos)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return v;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray aInputsArray = m_CurrentFilterIndex.value(name).toArray();
    if (aInputsArray.size() > vectorPos && aInputsArray[vectorPos].isObject())
    {
      QJsonObject aInputObject = aInputsArray[vectorPos].toObject();
      AxisAngleInput_t aInput;
      if (aInput.readJson(aInputObject) == true)
      {
        return aInput;
      }
    }
  }

  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<AxisAngleInput_t> JsonFilterParametersReader::readAxisAngles(const QString name, QVector<AxisAngleInput_t> v)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return v;
  }

  if (m_CurrentFilterIndex.value(name).isArray())
  {
    QJsonArray aInputsArray = m_CurrentFilterIndex.value(name).toArray();

    QVector<AxisAngleInput_t> inputs;
    for (int i = 0; i < aInputsArray.size(); i++)
    {
      if (aInputsArray[i].isObject() == true)
      {
        QJsonObject aInputObj = aInputsArray[i].toObject();
        AxisAngleInput_t aInput;
        if (aInput.readJson(aInputObj) == true)
        {
          inputs.push_back(aInput);
        }
        else
        {
          return v;
        }
      }
    }
    return inputs;
  }

  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSet<QString> JsonFilterParametersReader::readArraySelections(const QString name, QSet<QString> v)
{
  BOOST_ASSERT(m_CurrentFilterIndex.isEmpty() == false);
  if (m_CurrentFilterIndex.contains(name) == false)
  {
    return v;
  }

  QSet<QString> set;
  
  if (m_CurrentFilterIndex[name].isArray())
  {
    QJsonArray jsonArray = m_CurrentFilterIndex[name].toArray();

    foreach(QJsonValue val, jsonArray)
    {
      if (val.isString())
      {
        set.insert(val.toString());
      }
    }

    return set;
  }

  return v;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerArrayProxy JsonFilterParametersReader::readDataContainerArrayProxy(const QString& name, DataContainerArrayProxy v)
{
  //BOOST_ASSERT(m_Prefs != NULL);

  //DataContainerArrayProxy dcaProxy;
  //QString defValue;
  //int count = m_Prefs->beginReadArray(name);
  ////  QStringList paths;
  ////  QStringList flags;

  //for (int i = 0; i < count; i++)
  //{
  //  m_Prefs->setArrayIndex(i);
  //  DataArrayPath dap(m_Prefs->value("Path", defValue).toString());
  //  QString dcFlag = m_Prefs->value("DCFlag", defValue).toString();
  //  QString attrFlag = m_Prefs->value("ATTRFlag", defValue).toString();
  //  QString daFlag = m_Prefs->value("DAFlag", defValue).toString();
  //  // Add the data container proxy if it does not exist
  //  if (dcaProxy.contains(dap.getDataContainerName()) == false)
  //  {
  //    DataContainerProxy dcp(dap.getDataContainerName());
  //    if (dcFlag.compare("0") == 0)
  //    {
  //      dcp.flag = Qt::Unchecked;
  //    }
  //    else if (dcFlag.compare("1") == 0)
  //    {
  //      dcp.flag = Qt::PartiallyChecked;
  //    }
  //    else
  //    {
  //      dcp.flag = Qt::Checked;
  //    }
  //    dcaProxy.dataContainers.insert(dcp.name, dcp);
  //  }

  //  // Now we check for the AttributeMatrix
  //  if (dcaProxy.contains(dap.getDataContainerName()))
  //  {
  //    DataContainerProxy& dcProxy = dcaProxy.getDataContainerProxy(dap.getDataContainerName());
  //    if (dcProxy.attributeMatricies.find(dap.getAttributeMatrixName()) == dcProxy.attributeMatricies.end() && dap.getAttributeMatrixName().isEmpty() == false)
  //    {
  //      AttributeMatrixProxy attrProxy(dap.getAttributeMatrixName());
  //      if (attrFlag.compare("0") == 0)
  //      {
  //        attrProxy.flag = Qt::Unchecked;
  //      }
  //      else if (attrFlag.compare("1") == 0)
  //      {
  //        attrProxy.flag = Qt::PartiallyChecked;
  //      }
  //      else
  //      {
  //        attrProxy.flag = Qt::Checked;
  //      }
  //      dcProxy.attributeMatricies.insert(dap.getAttributeMatrixName(), attrProxy);
  //    }

  //    // Now we have the attribute matrix
  //    if (dcProxy.attributeMatricies.find(dap.getAttributeMatrixName()) != dcProxy.attributeMatricies.end())
  //    {
  //      AttributeMatrixProxy& attrProxy = dcProxy.attributeMatricies[dap.getAttributeMatrixName()];
  //      // Now check for the data array
  //      if (attrProxy.dataArrays.find(dap.getDataArrayName()) == attrProxy.dataArrays.end() && dap.getDataArrayName().isEmpty() == false)
  //      {
  //        DataArrayProxy proxy(QString("%1|%2").arg(dap.getDataContainerName()).arg(dap.getAttributeMatrixName()), dap.getDataArrayName());
  //        if (daFlag.compare("0") == 0)
  //        {
  //          proxy.flag = Qt::Unchecked;
  //        }
  //        else if (daFlag.compare("1") == 0)
  //        {
  //          proxy.flag = Qt::PartiallyChecked;
  //        }
  //        else
  //        {
  //          proxy.flag = Qt::Checked;
  //        }
  //        attrProxy.dataArrays.insert(dap.getDataArrayName(), proxy);
  //      }
  //    }
  //  }
  //}
  //m_Prefs->endArray();
  //return dcaProxy;

  return DataContainerArrayProxy();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataArrayPath JsonFilterParametersReader::readDataArrayPath(const QString& name, DataArrayPath def)
{
  /*BOOST_ASSERT(m_Prefs != NULL);
  if (m_Prefs->contains(name) == false)
  {
    return def;
  }
  QString defPath("");
  QString str = m_Prefs->value(name, defPath).toString();
  DataArrayPath path(str);
  return path;*/

  return DataArrayPath();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<DataArrayPath> JsonFilterParametersReader::readDataArrayPathVector(const QString& name, QVector<DataArrayPath> def)
{
  /*BOOST_ASSERT(m_Prefs != NULL);

  QVector<DataArrayPath> vector;

  int size = m_Prefs->beginReadArray(name);
  if (size <= 0)
  {
    return def;
  }

  for (int i = 0; i < size; ++i) {
    m_Prefs->setArrayIndex(i);
    QString pathStr = m_Prefs->value(DREAM3D::IO::DAPSettingsHeader).toString();
    DataArrayPath path = DataArrayPath::Deserialize(pathStr, "|");
    vector.append(path);
  }
  m_Prefs->endArray();

  return vector;*/

  return QVector<DataArrayPath>();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DynamicTableData JsonFilterParametersReader::readDynamicTableData(const QString& name, DynamicTableData def)
{
  /*BOOST_ASSERT(m_Prefs != NULL);

  QString dataStr;
  int numRows, numCols;
  QStringList rHeaders;
  QStringList cHeaders;

  int size = m_Prefs->beginReadArray(name);
  if (size <= 0)
  {
    m_Prefs->endArray();
    return def;
  }

  m_Prefs->setArrayIndex(0);
  dataStr = m_Prefs->value(name, "").toString();
  m_Prefs->setArrayIndex(1);
  rHeaders = m_Prefs->value(name, "").toStringList();
  m_Prefs->setArrayIndex(2);
  numRows = m_Prefs->value(name, 0).toInt();
  m_Prefs->setArrayIndex(3);
  numCols = m_Prefs->value(name, 0).toInt();
  m_Prefs->setArrayIndex(4);
  cHeaders = m_Prefs->value(name, "").toStringList();

  m_Prefs->endArray();

  std::vector<std::vector<double> > data = DynamicTableData::DeserializeData(dataStr, numRows, numCols, ',');

  DynamicTableData tableData(data, rHeaders, cHeaders);

  return tableData;*/

  return DynamicTableData();
}

