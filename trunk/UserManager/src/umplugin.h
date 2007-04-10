/*
    umplugin.h
    Copyright (C) 2001-2007 zg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

enum
{
  mName,
  mNameDisk,
  mDirRights,
  mDirAudit,
  mDirShared,
  mDirNewFolder,
  mDirNewFile,
  mDirNewKey,
  mDirNewContainer,
  mDirNewJob,

  mMenuProp,
  mMenuApply,
  mMenuSep1,
  mMenuUsers,
  mMenuRights,

  mTitleACL,
  mTitleUsers,
  mTitleRights,
  mTitleAccess,
  mTitleAudit,
  mTitleSID,
  mTitleMaximum,
  mTitleCurrent,
  mTitleLocalPath,

  mPropButtonOk,
  mPropButtonCancel,

  mPropFileReadData,
  mPropFileWriteData,
  mPropFileAppendData,
  mPropFileReadEA,
  mPropFileWriteEA,
  mPropFileExecute,
  mPropFileDeleteChild,
  mPropFileReadAttr,
  mPropFileWriteAttr,
  mPropFileDelete,
  mPropFileReadControl,
  mPropFileChangePermissions,
  mPropFileTakeOwnership,
  mPropFileSynchronize,
  mPropDirReadData,
  mPropDirWriteData,
  mPropDirAppendData,
  mPropDirReadEA,
  mPropDirWriteEA,
  mPropDirExecute,
  mPropDirDeleteChild,
  mPropDirReadAttr,
  mPropDirWriteAttr,
  mPropDirDelete,
  mPropDirReadControl,
  mPropDirChangePermissions,
  mPropDirTakeOwnership,
  mPropDirSynchronize,
  mPropSimpleFull,
  mPropSimpleSpecial,
  mPropSimpleRead,
  mPropSimpleWrite,
  mPropSimpleExecute,
  mPropSimpleDelete,
  mPropSimpleChangePermissions,
  mPropSimpleTakeOwnership,

  mPropRegQueryValue,
  mPropRegSetValue,
  mPropRegCreateSubkey,
  mPropRegEnumerateSubkeys,
  mPropRegNotify,
  mPropRegCreateLink,
  mPropRegDelete,
  mPropRegReadControl,
  mPropRegChangePermissions,
  mPropRegTakeOwnership,

  mPropPrintUse,
  mPropPrintAdminister,
  mPropJobAdminister,

  mDelFolderWarning,
  mDelOne,
  mDelACE1,
  mDelACE234,
  mDelACEN,
  mDelShare1,
  mDelShare234,
  mDelShareN,
  mDelUser1,
  mDelUser234,
  mDelUserN,
  mDelObject1,
  mDelObject234,
  mDelObjectN,
  mRemoveOne,
  mRemoveUser1,
  mRemoveUser234,
  mRemoveUserN,

  mShareNewShare,
  mShareShareName,
  mShareEditShare,
  mShareComment,
  mShareUserLimit,
  mShareMaximumAllowed,
  mShareAllow,
  mShareUsers,

  mGroupNewGroup,
  mGroupEditGroup,
  mGroupName,
  mGroupComment,

  mUserNew,
  mUserEdit,
  mUserProfileTitle,
  mUserUsername,
  mUserFullname,
  mUserDescription,
  mUserComment,
  mUserParams,
  mUserPassword1,
  mUserPassword2,
  mUserChk1,
  mUserChk2,
  mUserChk3,
  mUserChk4,
  mUserChk5,
  mUserButtonProfile,
  mUserProfile,
  mUserScript,
  mUserHome,
  mUserWorkstations,

  mA2CNoFolder,
  mA2CTitle,
  mA2CObjects,
  mA2CACL,
  mA2CFolders,
  mA2CFiles,
  mA2CKeys,
  mA2CRights,
  mA2CAudit,
  mA2COwner,
  mA2CProcessOne,
  mA2CProcess1,
  mA2CProcess234,
  mA2CProcessN,

  mSelCompTitle,
  mSelCompLabel,
  mSelCompFootnote,

  mConfigAddToDisksMenu,
  mConfigDisksMenuDigit,
  mConfigAddToPluginMenu,
  mConfigAddToConfigMenu,
  mConfigFullUserName,
  mConfigPrefix,
  mConfigSave,
  mConfigCancel,

  mButtonOk,
  mButtonCancel,
  mButtonCopy,
  mButtonDelete,
  mButtonRemove,

  mError,
  mErrorCantGetACL,

  mOtherDomainSearch,
  mOtherConnect,
  mOtherNetNotFound,
  mOtherPwdError1,
  mOtherPwdError2,
  mOtherPwdError3,

  mKeyEmpty,
  mKeyPerm,
  mKeyAdvPerm,
  mKeyAudit,
  mKeyAdvAudit,
  mKeyProperties,
  mKeyLocal,
  mKeyGlobal,
  mKeyAddShare,
  mKeyAddGroup,
  mKeyRemote,
  mKeyAddUser,
  mKeyRemove,
  mKeyChange,
  mKeyTakeOwnership,

  mPlainPanelType0,
  mPlainPanelSize0,
  mPlainColonType0,
  mPlainColonSize0,
  mPlainPanelFull0,
  mPlainPanelType1,
  mPlainPanelSize1,
  mPlainColonType1,
  mPlainColonSize1,
  mPlainPanelFull1,
  mPlainPanelType2,
  mPlainPanelSize2,
  mPlainColonType2,
  mPlainColonSize2,
  mPlainPanelFull2,
  mPlainPanelType3,
  mPlainPanelSize3,
  mPlainColonType3,
  mPlainColonSize3,
  mPlainPanelFull3,
  mPlainPanelType4,
  mPlainPanelSize4,
  mPlainColonType4,
  mPlainColonSize4,
  mPlainPanelFull4,
  mPlainPanelType5,
  mPlainPanelSize5,
  mPlainColonType5,
  mPlainColonSize5,
  mPlainPanelFull5,
  mPlainPanelType6,
  mPlainPanelSize6,
  mPlainColonType6,
  mPlainColonSize6,
  mPlainPanelFull6,
  mPlainPanelType7,
  mPlainPanelSize7,
  mPlainColonType7,
  mPlainColonSize7,
  mPlainPanelFull7,
  mPlainPanelType8,
  mPlainPanelSize8,
  mPlainColonType8,
  mPlainColonSize8,
  mPlainPanelFull8,
  mPlainPanelType9,
  mPlainPanelSize9,
  mPlainColonType9,
  mPlainColonSize9,
  mPlainPanelFull9,

  mPermPanelType0,
  mPermPanelSize0,
  mPermColonType0,
  mPermColonSize0,
  mPermPanelFull0,
  mPermPanelType1,
  mPermPanelSize1,
  mPermColonType1,
  mPermColonSize1,
  mPermPanelFull1,
  mPermPanelType2,
  mPermPanelSize2,
  mPermColonType2,
  mPermColonSize2,
  mPermPanelFull2,
  mPermPanelType3,
  mPermPanelSize3,
  mPermColonType3,
  mPermColonSize3,
  mPermPanelFull3,
  mPermPanelType4,
  mPermPanelSize4,
  mPermColonType4,
  mPermColonSize4,
  mPermPanelFull4,
  mPermPanelType5,
  mPermPanelSize5,
  mPermColonType5,
  mPermColonSize5,
  mPermPanelFull5,
  mPermPanelType6,
  mPermPanelSize6,
  mPermColonType6,
  mPermColonSize6,
  mPermPanelFull6,
  mPermPanelType7,
  mPermPanelSize7,
  mPermColonType7,
  mPermColonSize7,
  mPermPanelFull7,
  mPermPanelType8,
  mPermPanelSize8,
  mPermColonType8,
  mPermColonSize8,
  mPermPanelFull8,
  mPermPanelType9,
  mPermPanelSize9,
  mPermColonType9,
  mPermColonSize9,
  mPermPanelFull9,

  mSharePanelType0,
  mSharePanelSize0,
  mShareColonType0,
  mShareColonSize0,
  mSharePanelFull0,
  mSharePanelType1,
  mSharePanelSize1,
  mShareColonType1,
  mShareColonSize1,
  mSharePanelFull1,
  mSharePanelType2,
  mSharePanelSize2,
  mShareColonType2,
  mShareColonSize2,
  mSharePanelFull2,
  mSharePanelType3,
  mSharePanelSize3,
  mShareColonType3,
  mShareColonSize3,
  mSharePanelFull3,
  mSharePanelType4,
  mSharePanelSize4,
  mShareColonType4,
  mShareColonSize4,
  mSharePanelFull4,
  mSharePanelType5,
  mSharePanelSize5,
  mShareColonType5,
  mShareColonSize5,
  mSharePanelFull5,
  mSharePanelType6,
  mSharePanelSize6,
  mShareColonType6,
  mShareColonSize6,
  mSharePanelFull6,
  mSharePanelType7,
  mSharePanelSize7,
  mShareColonType7,
  mShareColonSize7,
  mSharePanelFull7,
  mSharePanelType8,
  mSharePanelSize8,
  mShareColonType8,
  mShareColonSize8,
  mSharePanelFull8,
  mSharePanelType9,
  mSharePanelSize9,
  mShareColonType9,
  mShareColonSize9,
  mSharePanelFull9,

  mUserPanelType0,
  mUserPanelSize0,
  mUserColonType0,
  mUserColonSize0,
  mUserPanelFull0,
  mUserPanelType1,
  mUserPanelSize1,
  mUserColonType1,
  mUserColonSize1,
  mUserPanelFull1,
  mUserPanelType2,
  mUserPanelSize2,
  mUserColonType2,
  mUserColonSize2,
  mUserPanelFull2,
  mUserPanelType3,
  mUserPanelSize3,
  mUserColonType3,
  mUserColonSize3,
  mUserPanelFull3,
  mUserPanelType4,
  mUserPanelSize4,
  mUserColonType4,
  mUserColonSize4,
  mUserPanelFull4,
  mUserPanelType5,
  mUserPanelSize5,
  mUserColonType5,
  mUserColonSize5,
  mUserPanelFull5,
  mUserPanelType6,
  mUserPanelSize6,
  mUserColonType6,
  mUserColonSize6,
  mUserPanelFull6,
  mUserPanelType7,
  mUserPanelSize7,
  mUserColonType7,
  mUserColonSize7,
  mUserPanelFull7,
  mUserPanelType8,
  mUserPanelSize8,
  mUserColonType8,
  mUserColonSize8,
  mUserPanelFull8,
  mUserPanelType9,
  mUserPanelSize9,
  mUserColonType9,
  mUserColonSize9,
  mUserPanelFull9,
};

enum
{
  levelRoot,
  levelFilePerm,
  levelFilePermNewFolder,
  levelFilePermNewFile,
  levelFileAudit,
  levelFileAuditFolder,
  levelFileAuditFile,
  levelRegRoot,
  levelRegRights,
  levelRegRightsNewKey,
  levelRegRightsNewPad,
  levelRegAudit,
  levelRegAuditKey,
  levelRegAuditPad,
  levelShared,
  levelSharedIn,
  levelSharedInAllowed,
  levelPrinterRoot,
  levelPrinterPerm,
  levelPrinterPermContainer,
  levelPrinterPermJob,
  levelPrinterAudit,
  levelPrinterAuditContainer,
  levelPrinterAuditJob,
  levelPrinterShared,
  levelGroups,
  levelUsers,
  levelRights,
  levelRightUsers,
};

enum
{
  actionUpdate,
  actionChangeType,
  actionInsert,
  actionDelete,
  actionMoveUp,
  actionMoveDown,
};

enum
{
  modePlain,
  modePermisions,
};

enum
{
  pathtypeReal,
  pathtypeTree,
  pathtypePlugin,
  pathtypeUnknown,
};

#define FLAG_FOLDER    0x00000001
#define FLAG_NETPATH   0x00000002

#define FAR165_INFO_SIZE 336
#define TINY_BUFFER 1024
#define SMALL_BUFFER 16*1024
#define BIG_BUFFER  128*1024
#define MAX_COMMENT 256
#define CUSTOM_COLUMN_COUNT 4

#define UM_ITEM_EMPTY        0
#define UM_ITEM_ALLOW        1
#define UM_ITEM_DENY         2
#define UM_ITEM_SUCCESS      3
#define UM_ITEM_FAIL         4
#define UM_ITEM_SUCCESS_FAIL 5

struct UserManager
{
  unsigned long flags;
  char path[MAX_PATH];
  char nonfixed_oem[MAX_PATH];
  char hostfile_oem[MAX_PATH];
  wchar_t nonfixed[MAX_PATH];
  wchar_t hostfile[MAX_PATH];
  wchar_t *computer_ptr;
  wchar_t computer[MAX_PATH];
  wchar_t domain[MAX_PATH]; //domain server or local path for network files
  unsigned long param;
  bool global;
  int level;
  bool error;
};

struct RegRoot
{
  wchar_t *root;
  HKEY value;
};

struct AceData
{
  unsigned long length;
  unsigned char ace_flags;
  unsigned char ace_type;
  unsigned long ace_mask;
  PSID user;
  AceData *next;
};

struct AclData
{
  AceData *Aces;
  unsigned long Count;
};

struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  char *Data;
};

struct WellKnownSID
{
  SID_IDENTIFIER_AUTHORITY sid_id[6];
  DWORD SubAuthority;
};

//options
struct Options
{
  bool AddToDisksMenu;
  int DisksMenuDigit;
  bool AddToPluginsMenu;
  bool AddToConfigMenu;
  bool FullUserNames;
  char Prefix[16];
};

extern struct Options Opt;

//callbacks
typedef PSECURITY_DESCRIPTOR (*GetSD)(UserManager *panel,SECURITY_INFORMATION si);
typedef bool (*SetSD)(UserManager *panel,SECURITY_INFORMATION si,PSECURITY_DESCRIPTOR SD);
typedef BOOL (WINAPI * GetSDAcl)(PSECURITY_DESCRIPTOR,LPBOOL,PACL*,LPBOOL);
typedef BOOL (WINAPI * SetSDAcl)(PSECURITY_DESCRIPTOR,BOOL,PACL,BOOL);
typedef bool (*CheckType)(unsigned char Param,unsigned char Type,unsigned char *Flags,unsigned long *Mask);
typedef bool (*PressButton)(UserManager *panel);
typedef bool (*PressButton2)(UserManager *panel,bool selection);
typedef bool (*PressButton3)(UserManager *panel,UserManager *anotherpanel,bool selection);
typedef char * (*ParseColumns)(char *columns);
typedef bool (*GetOwner)(UserManager *panel,PSID *sid,wchar_t **owner,char **owner_oem);

#define PERM_NO    0
#define PERM_YES   1
#define PERM_FF    2
#define PERM_KEY   3
#define PERM_PRINT 4

//arrays
extern const int up_dirs[];
extern const int root_dirs[];
extern const bool nonfixed_dirs[];
extern const bool has_nonfixed_dirs[];
extern const int *plain_dirs_dir[];
extern const GetOwner plain_dirs_owners[];
extern const unsigned char perm_dirs_dir[];
extern const SECURITY_INFORMATION security_information[];
extern const GetSD get_security_descriptor[];
extern const SetSD set_security_descriptor[];
extern const SetSDAcl set_security_descriptor_acl[];
extern const GetSDAcl get_security_descriptor_acl[];
extern const unsigned long reg_security_descriptor_access_read[];
extern const unsigned long reg_security_descriptor_access_write[];
extern const unsigned long printer_security_descriptor_access_read[];
extern const unsigned long printer_security_descriptor_access_write[];
extern const unsigned char ace_types[];
extern const unsigned char check_ace_type_param[];
extern const CheckType check_ace_type[];
extern const int relative_types[][5];
extern const bool has_hostfile[];
extern const unsigned char title_type[];
extern const int title_type_string[];
extern const bool press_f5_from[];
extern const PressButton press_f4[];
extern const PressButton press_alt_f4[];
extern const PressButton press_shift_f4[];
extern const PressButton3 press_f5[];
extern const PressButton2 press_f6[];
extern const PressButton press_f7[];
extern const PressButton2 press_f8[];
extern const unsigned char default_type[];
extern const unsigned char default_flags[];
extern const unsigned long default_mask[];
extern const int default_acl[];
extern const unsigned int *common_rights[];
extern const unsigned int common_full_access[];
extern const int panel_modes[];
extern const ParseColumns get_panel_titles[];
extern const int label_f4[];
extern const int label_alt_f4[];
extern const int label_shift_f4[];
extern const int label_f6[];
extern const int label_shift_f6[];
extern const int label_f7[];
extern const bool sort[];

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;
extern char PluginRootKey[80];

extern char *GetMsg(int MsgId);
extern void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,int ItemsNumber);
extern void ShowError(DWORD Error);
extern void ShowCustomError(int index);
extern void GetCurrentPath(int level,char *nonfixed,char *result);
extern DWORD generic_mask_to_file_mask(DWORD mask);
extern DWORD generic_mask_to_reg_mask(DWORD mask);
extern DWORD generic_mask_to_printer_mask(DWORD mask);
extern DWORD generic_mask_to_job_mask(DWORD mask);
extern void EnablePrivilege(const char *name);
extern bool IsPrivilegeEnabled(const char *name);
extern bool CheckChDir(HANDLE hPlugin,const char *NewDir,char *RealDir,wchar_t *RealDirW,int *level);
extern void AddDefaultUserdata(PluginPanelItem *Item,int level,int sortorder,int itemtype,PSID sid,wchar_t *wide_name,char *oem_name);
extern int GetLevelFromUserData(DWORD UserData);
extern int GetSortOrderFromUserData(DWORD UserData);
extern int GetItemTypeFromUserData(DWORD UserData);
extern PSID GetSidFromUserData(DWORD UserData);
extern wchar_t *GetWideNameFromUserData(DWORD UserData);
extern int NumberType(int num);
extern void GetSelectedList(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,bool selection);
extern PSECURITY_DESCRIPTOR CreateDefaultSD(void);
extern PACL CreateDefaultAcl(int level);
extern char *get_access_string(int level,int mask);
extern char *get_sid_string(PSID sid);
extern void wcsaddendslash(wchar_t *string);
extern bool GetWideName(char *root,const WIN32_FIND_DATAA *src,wchar_t *name);
extern int parse_dir(char *root_oem,char *obj_oem,wchar_t *obj,int obj_type,unsigned long *param,wchar_t *host,char *host_oem);

extern LONG RegOpenBackupKeyExW(HKEY hKey,LPCWSTR lpSubKey,REGSAM samDesired,PHKEY phkResult);

extern bool EditCommonAccess(UserManager *panel);

extern bool EditFileAdvancedAccess(UserManager *panel);
extern bool EditRegAdvancedAccess(UserManager *panel);
extern bool EditPrinterAdvancedAccess(UserManager *panel);
extern bool EditJobAdvancedAccess(UserManager *panel);

extern bool EditShareProperties(UserManager *panel);

extern bool AddOwner(UserManager *panel,UserManager *anotherpanel,bool selection);
extern bool AddACE(UserManager *panel,UserManager *anotherpanel,bool selection);
extern bool AddUserToGroup(UserManager *panel,UserManager *anotherpanel,bool selection);
extern bool AddUserToRight(UserManager *panel,UserManager *anotherpanel,bool selection);

extern bool AddOwnerInternal(UserManager *panel,PSID user);

extern bool ManageUser(UserManager *panel,bool type);
extern bool ManageGroup(UserManager *panel,bool type,wchar_t *in_group);

extern bool TakeOwnership(UserManager *panel,bool selection);
extern bool DeleteACE(UserManager *panel,bool selection);
extern bool DeleteShare(UserManager *panel,bool selection);
extern bool DeleteGroup(UserManager *panel,bool selection);
extern bool RemoveUser(UserManager *panel,bool selection);
extern bool DeleteUser(UserManager *panel,bool selection);
extern bool DeleteRightUsers(UserManager *panel,bool selection);

extern bool AddShare(UserManager *panel);

extern bool GetComputer(UserManager *panel,bool selection);

extern int GetAclState(int level,unsigned char type,unsigned long mask);
extern bool GetAcl(UserManager *panel,int level,AclData **data);
extern void FreeAcl(AclData *data);
extern bool UpdateAcl(UserManager *panel,int level,PSID user,int type,DWORD mask,int action);

extern void GetUserNameEx(wchar_t *computer,PSID sid,bool full,wchar_t **username,char **username_oem);
extern void free_sid_cache(void);

extern void ProcessChilds(PanelInfo *PInfo);

extern int Config(void);

extern void init_current_user(void);
extern bool is_current_user(PSID sid);
extern PSID current_user(void);
extern void free_current_user(void);

#include <ddk/ntifs.h>
