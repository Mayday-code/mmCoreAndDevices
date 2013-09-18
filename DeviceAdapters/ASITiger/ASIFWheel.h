///////////////////////////////////////////////////////////////////////////////
// FILE:          ASIFWheel.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI filter wheel adapter for Tiger
//
// COPYRIGHT:     Applied Scientific Instrumentation, Eugene OR
//
// LICENSE:       This file is distributed under the BSD license.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// AUTHOR:        Jon Daniels (jon@asiimaging.com) 09/2013
//
// BASED ON:      ASIStage.h and others
//

#ifndef _ASIFWheel_H_
#define _ASIFWheel_H_

#include "ASIDevice.h"
#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"

using namespace std;

class CFWheel : public CStateDeviceBase<CFWheel>, ASIDevice
{
public:
   CFWheel(const char* name);
   ~CFWheel();
  
   // Generic device API
   // ----------
   int Initialize();
   bool Busy();
   int Shutdown() { return ASIDevice::Shutdown(); }
   void GetName(char* pszName) const { ASIDevice::GetName(pszName); }

   // State device API
   // -----------
   unsigned long GetNumberOfPositions() const { return numPositions_; }

   // action interface
   // ----------------
   int OnState       (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLabel       (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSpin        (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnVelocity    (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSpeedSetting(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLockMode    (MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   unsigned int numPositions_;
   unsigned int curPosition_;
   unsigned int spinning_;
   string axisLetter_;

   int SelectWheel();
};

#endif //_ASIFWheel_H_