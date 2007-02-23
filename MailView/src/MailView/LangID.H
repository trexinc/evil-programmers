/*
 MailView plugin for FAR Manager
 Copyright (C) 2005 Alex Yaroslavsky
 Copyright (C) 2002-2003 Dennis Trachuk

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef ___LangID_H___
#define ___LangID_H___

enum TMessageStrings
  {
    MOk,
    MCancel,
    MYes,
    MNo,

    MMailView,

    MInetMessage,
    MFidoMessage,
    MInetNews,
    MWebArchive,

    MPanelTitle_Format,

    MMailbox_FormatName,
    MMailbox_Unknown,

    MSubject,
    MFrom,
    MTo,
    MCc,
    MBcc,
    MSent,
    MReceived,
    MAccessed,
    MSize,
    MPriority,
    MFlags,

    MArea,
    MNewsgroups,
    MOrganisation,
    MMailer,
    MMessageId,

    MPriorityNormal,
    MPriorityHigh,
    MPriorityLow,

    MReadingMailbox,
    MFoundNMessages,
    MReadingMailboxCache,

    // sort modes menu
    MsmMnuSortBy,
    MsmMnuSubject,
    MsmMnuFrom,
    MsmMnuTo,
    MsmMnuSize,
    MsmMnuUnsorted,
    MsmMnuCreationTime,
    MsmMnuReceivedTime,
    MsmMnuAccessedTime,
    MsmMnuViewThreadBy,
    MvtMnuNone,
    MvtMnuReferences,
    MvtMnuSubject,
    MvtMnuFrom,
    MvtMnuTo,
    MvtMnuReferencesExt,

    // sort modes key bar (ctrl)
    MkbCtrlF1,
    MkbCtrlF2,
    MkbCtrlF3,
    MkbCtrlF4,
    MkbCtrlF5,
    MkbCtrlF6,
    MkbCtrlF7,
    MkbCtrlF8,
    MkbCtrlF9,
    MkbCtrlF10,
    MkbCtrlF11,
    MkbCtrlF12,

    // key bar (shift)
    MkbShiftF1,
    MkbShiftF2,
    MkbShiftF3,
    MkbShiftF4,
    MkbShiftF5,
    MkbShiftF6,
    MkbShiftF7,
    MkbShiftF8,
    MkbShiftF9,
    MkbShiftF10,
    MkbShiftF11,
    MkbShiftF12,

    // color selection dialog
    MColorDlg_Color,

    MColorDlg_Foreground,
    MColorDlg_Background,

    MColorDlg_Set,

    // color configuration menu
    MCfg_clNormal,
    MCfg_clBold,
    MCfg_clItalic,
    MCfg_clUnderline,
    MCfg_clBoldItalic,
    MCfg_clBoldUnderline,
    MCfg_clItalicUnderline,
    MCfg_clBoldItalicUnderline,
    MCfg_clReverse,
    MCfg_clReverseBold,
    MCfg_clReverseItalic,
    MCfg_clReverseUnderline,
    MCfg_clReverseBoldItalic,
    MCfg_clReverseBoldUnderline,
    MCfg_clReverseItalicUnderline,
    MCfg_clReverseBoldItalicUnderline,
    MCfg_clKludges,
    MCfg_clQuotesEven,
    MCfg_clQuotesOdd,
    MCfg_clSignature,
    MCfg_clOrigin,
    MCfg_clTearline,
    MCfg_clTagline,
    Mcfg_clHyperlink,
    MCfg__Window,
    MCfg_clTitle,
    MCfg_clBorder,
    MCfg_clHeaderLines,
    MCfg_clHeaderEdit,
    MCfg_clHeaderEditSel,
    MCfg_clDateTime,

    MDlgSearch,

    MMailViewConfiguration,
    MViewer,
    MColors,
    MMessages,
    MNews,
    MWebArchives,
    MMailboxes,

    MTables,
    MtblDefault,
    MtblDos,
    MtblWindows,
    MtblAutodetect,
    MtblMore,

    MFileMasks,
    MReset,

    MdlgCopy_Copy,
    MdlgCopy_CopyItems,
    MdlgCopy_CopyItem,
    MdlgCopy_btnCopy,
    MdlgCopy_btnTree,
    MdlgCopy_btnCancel,
    MdlgCopy_CopyingTheFile,
    MdlgCopy_To,
    MdlgCopy_In,
    MdlgCopy_Total,
    MdlgCopy_TimeElapsedAndLeft,

    MWarning,
    MFileAlreadyExists,
    MNew,
    MExisting,
    MOverwrite,
    MAll,
    MSkip,
    MSkipAll,
    MAppend,

    MConfirm,
    MAbortOperation,

    MCheckAndOpen,
    MConfirmFileOpen,
    MFileInfected,

    MHyperViewConfiguration,
    MLibFullFileName,
    MCfgFullFileName,
    MEnabled,

    MMailbox_ConfigureS,
    MMailbox_DefailtEncoding,

    MViewCfg_HyperView,
    MViewCfg_Templates,

    MViewCfg_Templates_Editor,
    MViewCfg_Templates_Viewer,
    MViewCfg_Templates_QuickView,

    MAttributes,
    MAttributes_Readed,
    MAttributes_Flagged,
    MAttributes_Replied,
    MAttributes_Forwarded,
    MAttributes_Priority,
    MAttributes_Priority_Normal,
    MAttributes_Priority_High,
    MAttributes_Priority_Low,
    MAttributes_Encoding,

};

#endif //!defined(___LangID_H___)
