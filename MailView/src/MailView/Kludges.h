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
#ifndef ___Kludges_H___
#define ___Kludges_H___

// std
#define K_RFC_Subject                 "Subject"
#define K_RFC_Date                    "Date"
#define K_RFC_Received                "Received"
// adress
#define K_RFC_From                    "From"
#define K_RFC_To                      "To"
#define K_RFC_Cc                      "Cc"
#define K_RFC_Bcc                     "Bcc"
#define K_RFC_ReplyTo                 "Reply-To"
#define K_RFC_ReturnPath              "Return-Path"
#define K_RFC_DeliveredTo             "Delivered-To"
#define K_RFC_XCommentTo              "X-Comment-To"
#define K_RFC_XEnvelopeTo             "X-Envelope-To"
// mime
#define K_RFC_MimeVersion             "MIME-Version"
#define K_RFC_ContentType             "Content-Type"
#define K_RFC_ContentTransferEncoding "Content-Transfer-Encoding"
#define K_RFC_ContentDisposition      "Content-Disposition"
#define K_RFC_ContentLocation         "Content-Location"
#define K_RFC_XContentFile            "X-Content-File"
//
#define K_RFC_InReplyTo               "In-Reply-To"
#define K_RFC_References              "References"
#define K_RFC_XRusnewsReplyId         "X-Rusnews-Reply-Id"
//
#define K_RFC_Organization            "Organization"
#define K_RFC_MessageId               "Message-ID"
#define K_RFC_XPriority               "X-Priority"
#define K_RFC_XMailer                 "X-Mailer"
#define K_RFC_Newsgroups              "Newsgroups"
// ftn
#define K_RFC_XFTNMsgId               "X-FTN-MSGID"
#define K_RFC_XFTNArea                "X-FTN-AREA"
#define K_RFC_XFTNOrigin              "X-FTN-Origin"
#define K_RFC_XFTNTearline            "X-FTN-Tearline"

#define K_FTN_Area                    "AREA"
#define K_FTN_FMPT                    "\1FMPT\x20"
#define K_FTN_TOPT                    "\1TOPT\x20"
#define K_FTN_INTL                    "\1INTL\x20"
#define K_FTN_MsgId                   "\1MSGID"
#define K_FTN_Reply                   "\1REPLY"
#define K_FTN_ReplyAddr               "\1REPLYADDR"
#define K_FTN_ReplyTo                 "\1REPLYTO"
#define K_FTN_CHRS                    "\1CHRS"
#define K_FTN_TID                     "\1TID"
#define K_FTN_PID                     "\1PID"


#endif // !defined(___Kludges_H___)
