/** @file
  SMM USB Dispatch2 Protocol as defined in PI 1.1 Specification
  Volume 4 System Management Mode Core Interface.

  Provides the parent dispatch service for the USB SMI source generator.

  Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Revision Reference:
  This protocol is from PI Version 1.1.

**/

#ifndef _SMM_USB_DISPATCH2_H_
#define _SMM_USB_DISPATCH2_H_

#include <uefi/edk/Pi/PiSmmCis.h>

#define EFI_SMM_USB_DISPATCH2_PROTOCOL_GUID \
  { \
    0xee9b8d90, 0xc5a6, 0x40a2, {0xbd, 0xe2, 0x52, 0x55, 0x8d, 0x33, 0xcc, 0xa1 } \
  }

///
/// USB SMI event types
///
typedef enum {
  UsbLegacy,
  UsbWake
} EFI_USB_SMI_TYPE;

///
/// The dispatch function's context.
///
typedef struct {
  ///
  /// Describes whether this child handler will be invoked in response to a USB legacy 
  /// emulation event, such as port-trap on the PS/2* keyboard control registers, or to a 
  /// USB wake event, such as resumption from a sleep state.
  ///
  EFI_USB_SMI_TYPE          Type;
  ///
  /// The device path is part of the context structure and describes the location of the 
  /// particular USB host controller in the system for which this register event will occur.
  /// This location is important because of the possible integration of several USB host 
  /// controllers in a system.
  ///
  EFI_DEVICE_PATH_PROTOCOL  *Device;
} EFI_SMM_USB_REGISTER_CONTEXT;

typedef struct _EFI_SMM_USB_DISPATCH2_PROTOCOL EFI_SMM_USB_DISPATCH2_PROTOCOL;

/**
  Provides the parent dispatch service for the USB SMI source generator.

  This service registers a function (DispatchFunction) which will be called when the USB-
  related SMI specified by RegisterContext has occurred. On return, DispatchHandle 
  contains a unique handle which may be used later to unregister the function using UnRegister().
  The DispatchFunction will be called with Context set to the same value as was passed into 
  this function in RegisterContext and with CommBuffer containing NULL and 
  CommBufferSize containing zero.

  @param[in]  This               Pointer to the EFI_SMM_USB_DISPATCH2_PROTOCOL instance.
  @param[in]  DispatchFunction   Function to register for handler when a USB-related SMI occurs. 
  @param[in]  RegisterContext    Pointer to the dispatch function's context.
                                 The caller fills this context in before calling
                                 the register function to indicate to the register
                                 function the USB SMI types for which the dispatch
                                 function should be invoked.
  @param[out] DispatchHandle     Handle generated by the dispatcher to track the function instance.

  @retval EFI_SUCCESS            The dispatch function has been successfully
                                 registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR       The driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER  RegisterContext is invalid. The USB SMI type
                                 is not within valid range.
  @retval EFI_OUT_OF_RESOURCES   There is not enough memory (system or SMM) to manage this child.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SMM_USB_REGISTER2)(
  IN  CONST EFI_SMM_USB_DISPATCH2_PROTOCOL  *This,
  IN        EFI_SMM_HANDLER_ENTRY_POINT2    DispatchFunction,
  IN  CONST EFI_SMM_USB_REGISTER_CONTEXT    *RegisterContext,
  OUT       EFI_HANDLE                      *DispatchHandle
  );

/**
  Unregisters a USB service.

  This service removes the handler associated with DispatchHandle so that it will no longer be 
  called when the USB event occurs.

  @param[in]  This               Pointer to the EFI_SMM_USB_DISPATCH2_PROTOCOL instance.
  @param[in]  DispatchHandle     Handle of the service to remove. 

  @retval EFI_SUCCESS            The dispatch function has been successfully
                                 unregistered and the SMI source has been disabled
                                 if there are no other registered child dispatch
                                 functions for this SMI source.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SMM_USB_UNREGISTER2)(
  IN CONST EFI_SMM_USB_DISPATCH2_PROTOCOL  *This,
  IN       EFI_HANDLE                      DispatchHandle
  );

///
/// Interface structure for the SMM USB SMI Dispatch2 Protocol
///
/// This protocol provides the parent dispatch service for the USB SMI source generator.
///
struct _EFI_SMM_USB_DISPATCH2_PROTOCOL {
  EFI_SMM_USB_REGISTER2    Register;
  EFI_SMM_USB_UNREGISTER2  UnRegister;
};

extern EFI_GUID gEfiSmmUsbDispatch2ProtocolGuid;

#endif

