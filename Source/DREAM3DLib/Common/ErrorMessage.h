/* ============================================================================
 * Copyright (c) 2012, Michael A. Jackson (BlueQuartz Software)
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
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef _ERRORMESSAGE_H_
#define _ERRORMESSAGE_H_

#include <string>



#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"

#if defined(QT_CORE_LIB)
#include <QtCore/QMetaType>
#endif

/**
 * @class ErrorMessage ErrorMessage.h DREAM3DLib/Common/ErrorMessage.h
 * @brief
 * @author Joey Kleingers (SOCHE)
 * @date Jun 21, 2012
 * @version 1.0
 */
class ErrorMessage
{
  public:

    DREAM3D_SHARED_POINTERS(ErrorMessage)

    ErrorMessage() {}

    explicit ErrorMessage(const ErrorMessage& rhs)
    {

    }
    void operator=(const ErrorMessage& rhs)
    {

    }

    ErrorMessage(const std::string &m_filterName, const std::string &m_errorDescription, int m_errorCode) :
      m_filterName(m_filterName),
      m_errorDescription(m_errorDescription),
      m_errorCode(m_errorCode)
  {}


    DREAM3D_STATIC_NEW_MACRO(ErrorMessage)

    static Pointer New(const std::string &m_filterName, const std::string &m_errorDescription, int m_errorCode) {
      Pointer sharedPtr(new ErrorMessage(m_filterName, m_errorDescription, m_errorCode));
      return sharedPtr;
    }

    DREAM3D_TYPE_MACRO(ErrorMessage)

    virtual ~ErrorMessage() {}

    /**
     * @brief
     * @return
     */
    std::string getFilterName() { return m_filterName; }
    /**
     * @brief
     * @param val
     */
    void setFilterName(std::string val) { m_filterName = val; }

    std::string getErrorDescription() { return m_errorDescription; }
    void setErrorDescription(std::string val) { m_errorDescription = val; }

    int getErrorCode() { return m_errorCode; }
    void setErrorCode(int val) { m_errorCode = val; }

    std::string generateErrorString()
    {
      std::stringstream ss;
      ss << "Error(" << m_errorCode << "):" << m_filterName << " :" << m_errorDescription;
      return ss.str();
    }
    std::string generateWarningString()
    {
      std::stringstream ss;
      ss << "Warning(" << m_errorCode << "):" << m_filterName << " :" << m_errorDescription;
      return ss.str();
    }
    std::string generateStatusString()
     {
       std::stringstream ss;
       ss << m_filterName << ":" << m_errorDescription;
       return ss.str();
     }

  protected:



  private:
    std::string m_filterName;
    std::string m_errorDescription;
    int m_errorCode;


};

#if defined(QT_CORE_LIB)
// insert the custom custom Qt signal/slot macro dohicky thingie here
Q_DECLARE_METATYPE(ErrorMessage);
#endif

#endif /* _ErrorMessage_H */
