/* $Header: /cvsroot/farplus/FARPlus/FARXml.h,v 1.1 2002/04/14 10:39:29 yole Exp $
   FAR+Plus: lightweight XML parser interface
   (C) 2002 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARXML_H
#define __FARXML_H

#include "FARString.h"
#include "FARArray.h"

class IFarXMLErrorSink
{
public:
	virtual void ReportError (int line, int col, const char *expected) = 0;
};

class FarXMLNode
{
private:
	class Attribute
	{
	public:
		FarString fName;
		FarString fValue;
		
		Attribute (const FarString &name, const FarString &value)
			: fName (name), fValue (value) {};
	};

	FarString fTagName;
	FarArray<Attribute> fAttributes;
	FarArray<FarXMLNode> fChildren;
	int fStartLine;
	int fStartCol;
	int fEndLine;
	int fEndCol;

public:
	FarXMLNode()
		: fStartLine (-1), fStartCol (-1), fEndLine (-1), fEndCol (-1)
	{
	}

	// -- accessors ----------------------------------------------------------

    FarString GetTag() const
	{
        return fTagName;
	}

	int GetChildCount() const
	{
		return fChildren.Count();
	}

	FarXMLNode *GetChild (int index) const
	{
		if (index < 0 || index >= fChildren.Count())
			return NULL;
		return fChildren [index];
	}

	int GetAttrCount() const
	{
		return fAttributes.Count();
	}

	FarString GetAttr (const char *attrName) const
	{
		for (int i=0; i<fAttributes.Count(); i++)
		{
			if (fAttributes [i]->fName == attrName)
				return fAttributes [i]->fValue;
		}
		return FarString();
	}

	int GetStartLine() const
		{ return fStartLine; }

	int GetStartCol() const
		{ return fStartCol; }

	int GetEndLine() const
		{ return fEndLine; }

	int GetEndCol() const
		{ return fEndCol; }

	// -- modifiers -------------------------------------------------------------
	
	void SetTag (const FarString &tagName)
	{
		fTagName = tagName;
	}

	void AddAttr (const FarString &name, const FarString &value)
	{
        fAttributes.Add (new Attribute (name, value));
	}

	void AddChild (FarXMLNode *node)
	{
		fChildren.Add (node);
	}

	void SetStartPosition (int startLine, int startCol)
	{
		fStartLine = startLine;
		fStartCol  = startCol;
	}

	void SetEndPosition (int endLine, int endCol)
	{
		fEndLine = endLine;
		fEndCol  = endCol;
	}
};

namespace FarXML
{
	FarXMLNode *ParseFile (const char *FileName, IFarXMLErrorSink *errorSink = NULL);
	FarXMLNode *ParseBuffer (const char *Buf, IFarXMLErrorSink *errorSink = NULL);
}

#endif
