#ifndef __BMC_LAN_PARAM_H__
#define __BMC_LAN_PARAM_H__

#include <Uefi.h>

#define BmcLanConfig_Live_status 1


typedef struct {
  /// Callback.
  EFI_HII_CONFIG_ACCESS_PROTOCOL  Callback;
  UINT16                          Class;      ///< Formset class.
  UINT16                          SubClass;   ///< Formset sub-class.
} SETUP_CALLBACK;

typedef struct {
  EFI_HII_CONFIG_ACCESS_PROTOCOL* This;
  EFI_BROWSER_ACTION Action;
  EFI_QUESTION_ID KeyValue;
  UINT8 Type;
  EFI_IFR_TYPE_VALUE* Value;
  EFI_BROWSER_ACTION_REQUEST* ActionRequest;
} CALLBACK_PARAMETERS;


EFI_STATUS
BmcLanConfigCallbackFunction(
  IN  EFI_HII_HANDLE     HiiHandle,
  IN  UINT16             Key,
  IN CALLBACK_PARAMETERS* CallbackParameters);


EFI_STATUS
BmcLanIpv6ConfigCallbackFunction(
  IN EFI_HII_HANDLE     HiiHandle,
  IN UINT16             Key,
  CALLBACK_PARAMETERS* CallbackParameters);

EFI_STATUS
BmcLanIpv6Router1ConfigCallbackFunction(
  IN  EFI_HII_HANDLE    HiiHandle,
  IN  UINT16            Key,
  CALLBACK_PARAMETERS* CallbackParameters);

EFI_STATUS
BmcLanIpv6Router1PrefixValueCallbackFunction(
  IN  EFI_HII_HANDLE    HiiHandle,
  IN  UINT16            Key,
  CALLBACK_PARAMETERS* CallbackParameters);

EFI_STATUS
BmcLanIpv6SupportCallbackFunction(
  IN  EFI_HII_HANDLE     HiiHandle,
  IN  UINT16             Key,
  CALLBACK_PARAMETERS* CallbackParameters);

#if (BmcLanConfig_Live_status)
EFI_STATUS
BmcLanParamSetupDisplayCallback(
  IN EFI_HII_HANDLE     HiiHandle,
  IN UINT16             Key,
  CALLBACK_PARAMETERS* CallbackParameters);
#endif

#if (BmcLanConfig_Live_status == 0)
EFI_STATUS
BmcLanParamSetupDisplay(
  IN EFI_HII_HANDLE HiiHandle);
#endif


EFI_STATUS
BmcLanIPValidation(
  IN  EFI_HII_HANDLE    HiiHandle,
  IN  UINT16            Key,
  CALLBACK_PARAMETERS* CallbackParameters);

#endif
