/* $Header: /cvsroot/farplus/FARPlus/FARXml.cpp,v 1.1 2002/04/14 10:39:29 yole Exp $
   FAR+Plus: lightweight XML parser implementation
   (C) 2002 Dmitry Jemerov <yole@yole.ru>
*/

#include "FARXml.h"
#include "FARFile.h"
#include "FARPlus.h"

class FarXMLScanner
{
private:
  enum
  {
    CF_NAMESTART  = 1,
    CF_NAMECHAR   = 2,
    CF_WHITESPACE = 4
  };

  char fCharFlags [256];
  const char *fBuffer;
  const char *fBufPos;
  const char *fCurLineStart;
  int fCurLine;
  IFarXMLErrorSink *fErrorSink;

  void InitCharFlags();

  bool IsNameStart (char c)
  {
    return (fCharFlags [c] & CF_NAMESTART) != 0;
  }

  bool IsNameChar (char c)
  {
    return (fCharFlags [c] & CF_NAMECHAR) != 0;
  }

  bool HasNext()
  {
    return *fBufPos != '\0';
  }

    void SkipWhitespace()
  {
    while (fCharFlags [*fBufPos] & CF_WHITESPACE)
      NextChar();
  }

  char NextChar()
  {
    char c = *fBufPos++;
    if (c == '\n')
      fCurLine++;
    if (c == '\n' || c == '\r')
      fCurLineStart = fBufPos;
    return c;
  }

  int GetCurColumn() const
  {
    return fBufPos - fCurLineStart + 1;
  }

  char PeekNextChar (int delta = 0)
  {
    return fBufPos [delta];
  }

  bool NextExpectedChar (char expected)
  {
    char c = NextChar();
    if (c != expected)
    {
      if (fErrorSink != NULL)
      {
        char buf [2];
        buf [0] = expected;
        buf [1] = '\0';
        fErrorSink->ReportError (fCurLine, GetCurColumn(), buf);
      }
      return false;
    }
    return true;
  }

  FarString NextName();

  void ReportError (const char *expected)
  {
    if (fErrorSink)
            fErrorSink->ReportError (fCurLine, GetCurColumn(), expected);
  }

  bool ParseAttribute (FarXMLNode *node);
  int ParseEntity();
  int DecodeEntity (const FarString &entityName);

public:
  FarXMLScanner (const char *buf, IFarXMLErrorSink *errorSink)
    : fBuffer       (buf),
      fBufPos       (buf),
      fCurLineStart (buf),
      fCurLine      (1),
      fErrorSink    (errorSink)
  {
    InitCharFlags();
  }

  FarXMLNode *ParseChildNode();
};

// -- FarXMLScanner implementation -------------------------------------------

void FarXMLScanner::InitCharFlags()
{
  for (int i=0; i<256; i++)
  {
    fCharFlags [i] = 0;
    if (FarSF::LIsAlpha (i) || i == '_' || i == ':')
            fCharFlags [i] |= CF_NAMESTART;
    if (FarSF::LIsAlphanum (i) || i == '.' || i == '-' || i == '_' || i == ':')
      fCharFlags [i] |= CF_NAMECHAR;
  }
  fCharFlags [' ']  |= CF_WHITESPACE;
  fCharFlags ['\t'] |= CF_WHITESPACE;
  fCharFlags ['\n'] |= CF_WHITESPACE;
  fCharFlags ['\r'] |= CF_WHITESPACE;
}

FarString FarXMLScanner::NextName()
{
    if (!IsNameStart (*fBufPos))
  {
    if (fErrorSink)
      fErrorSink->ReportError (fCurLine, GetCurColumn(), "name");
    return FarString();
  }
  int nameLength = 1;
  while (IsNameChar (fBufPos [nameLength]))
    nameLength++;
    FarString result (fBufPos, nameLength);
  fBufPos += nameLength;
  return result;
}

FarXMLNode *FarXMLScanner::ParseChildNode()
{
    SkipWhitespace();

  int tagStartLine = fCurLine;
  int tagStartCol = GetCurColumn();
  if (!NextExpectedChar ('<'))
    return NULL;

  FarString tagName = NextName();
    if (tagName.IsEmpty())
    return NULL;

    FarXMLNode *theNode = new FarXMLNode();
  theNode->SetTag (tagName);
  theNode->SetStartPosition (tagStartLine, tagStartCol);

  while (1)
  {
    SkipWhitespace();
    char c = PeekNextChar();
    if (c == '/')
    {
      NextChar();
      if (!NextExpectedChar ('>'))
      {
        delete theNode;
        return NULL;
      }
      break;
    }
    else if (c == '>')
    {
      NextChar();
      SkipWhitespace();
      while (1)
      {
        if (PeekNextChar() == '<' && PeekNextChar (1) == '/')
          break;
        FarXMLNode *nextChild = ParseChildNode();
        if (!nextChild)
        {
          delete theNode;
          return NULL;
        }
        theNode->AddChild (nextChild);
        SkipWhitespace();
      }
      NextChar(); // <
      NextChar(); // /
      FarString closeTagName = NextName();
      if (closeTagName != tagName)
      {
        ReportError (tagName);
        delete theNode;
        return NULL;
      }
      SkipWhitespace();
      if (!NextExpectedChar ('>'))
      {
        delete theNode;
        return NULL;
      }
      break;
    }
    else
    {
      if (!ParseAttribute (theNode))
      {
        delete theNode;
        return NULL;
      }
    }
  }
  theNode->SetEndPosition (fCurLine, GetCurColumn());
  SkipWhitespace();

  return theNode;
}

bool FarXMLScanner::ParseAttribute (FarXMLNode *node)
{
    FarString attrName = NextName();
  if (attrName.IsEmpty())
    return false;
    SkipWhitespace();
  if (!NextExpectedChar ('='))
    return false;
  SkipWhitespace();

  char valueDelimiter = NextChar();
  if (valueDelimiter != '\'' && valueDelimiter != '\"')
  {
    ReportError ("\" or \'");
    return false;
  }

  FarString attrValue;
  while (1)
  {
    char c = NextChar();
    if (c == valueDelimiter)
      break;
    else if (c == '&')
    {
      int entityValue = ParseEntity();
      if (entityValue == -1)
        return false;
      attrValue += (char) entityValue;
    }
    else
            attrValue += c;
  }
  node->AddAttr (attrName, attrValue);
  return true;
}

int FarXMLScanner::ParseEntity()
{
  FarString entityName;
  while (1)
  {
    char c = NextChar();
    if (c == ';')
      break;
    entityName += c;
  }
  if (entityName.IsEmpty())
  {
    ReportError ("entity name");
    return -1;
  }
  if (entityName [0] == '#')
    return DecodeEntity (entityName);
  if (entityName == "quot")
    return '\"';
  if (entityName == "apos")
    return '\'';
  if (entityName == "gt")
    return '<';
  if (entityName == "lt")
    return '>';
  if (entityName == "amp")
    return '&';
  ReportError ("known entity name");
  return -1;
}

int FarXMLScanner::DecodeEntity (const FarString &entityName)
{
  int startPos = 1;
  int radix = 10;
  if (entityName [1] == 'x')
  {
    radix = 16;
    startPos++;
  }

  int value = 0;
  for (int pos=startPos; pos<entityName.Length(); pos++)
  {
    value *= radix;
    char c = entityName [pos];
        if (c >= '0' && c <= '9')
      value += c - '0';
    else if (radix == 16 && c >= 'A' && c <= 'F')
      value += c - 'A' + 10;
    else if (radix == 16 && c >= 'a' && c <= 'f')
      value += c - 'a' + 10;
    else
    {
      if (radix == 16)
        ReportError ("hex digit");
      else
        ReportError ("digit");
      return -1;
    }
  }
  return value;
}

namespace FarXML
{
  FarXMLNode *ParseFile (const char *FileName, IFarXMLErrorSink *errorSink)
  {
    FarFile f;
    if (!f.OpenForRead (FileName))
      return NULL;

    int fileSize = f.GetSize();
    FarString xmlBuf;
    xmlBuf.SetLength (fileSize);
    f.Read (xmlBuf.GetBuffer(), fileSize);

    return ParseBuffer (xmlBuf, errorSink);
  }

  FarXMLNode *ParseBuffer (const char *Buf, IFarXMLErrorSink *errorSink)
  {
    FarXMLScanner scanner (Buf, errorSink);

    return scanner.ParseChildNode();
  }
}
