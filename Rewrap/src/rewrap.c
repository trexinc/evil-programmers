#include <plugin.hpp>
#include <CRT/crt.hpp>

enum{
    IDS_Rewrap,
    IDS_Cancel,
    IDS_NoEsc,
    IDS_OldEsc,
    IDS_NoWrap,
    IDS_ReformatParagraph
};

static const wchar_t szEsc[]=L"ESC";

int ModuleNumber;
FARAPIEDITORCONTROL EditorControl;
FARAPIGETMSG        GetMsg;
FARAPIMESSAGE       Message;

// configurable parameters
int nWrapPos=0;            // 0 for no wrap
int isJustifyEnabled=0;
int isWrapQuote=0;         // process quotes

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
    ModuleNumber=Info->ModuleNumber;
    EditorControl=Info->EditorControl;
    GetMsg=Info->GetMsg;
    Message=Info->Message;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
    static const wchar_t* PluginMenuStrings;

    PluginMenuStrings=GetMsg(ModuleNumber,IDS_ReformatParagraph);

    Info->StructSize=sizeof(*Info);
    Info->Flags=PF_DISABLEPANELS|PF_EDITOR;
    Info->PluginMenuStrings=&PluginMenuStrings;
    Info->PluginMenuStringsNumber=1;
}

struct EditorInfo ei;
wchar_t nlsSpace;

inline int IsCSpace(wchar_t ch)
{
    return ch==L' ';
}

static int IsQuote(const wchar_t* pszStr, size_t nLength)
// returns the length of initial string part which is a quote or 0 otherwize
{
    size_t i=0;
    wchar_t q=L'>';

    while( i<nLength && i<4 ){
        if( pszStr[i++]==q ){
            while( i<nLength && (pszStr[i]==q || IsCSpace(pszStr[i])) ) i++;
            return i;
        }
    }
    return 0;
}

static int IsSameQuote(const wchar_t* pszQuote1, size_t nLen1, const wchar_t* pszQuote2, size_t nLen2)
{
    while( nLen1 )
        if( IsCSpace(pszQuote1[nLen1-1]) )nLen1--;
        else break;
    while( nLen2 )
        if( IsCSpace(pszQuote2[nLen2-1]) )nLen2--;
        else break;
    return nLen1==nLen2 && memcmp(pszQuote1,pszQuote2,nLen1*sizeof(wchar_t))==0;
}

HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item)
{
    struct EditorSetPosition esp;
    struct EditorGetString egs;
    struct EditorSetString ess;
    struct EditorSelect es;
    struct EditorUndoRedo eur={0};
    int i,j;
    int nIndent1, nIndent2;
    wchar_t* pMem;
    int nLen;
    int isBlank;
    int nStart;
    int nPara;
    int nAddLine;
    div_t SpaceCount;
    static HMODULE hEsc=NULL;
    static int (WINAPI *GetEditorSettings)(int EditorID, const wchar_t *szName, void *Param);
    const wchar_t* szText[3];
    int nQuote;
    wchar_t* szQuote;
    HANDLE hHeap;

    (void)OpenFrom;
    (void)Item;

    if( !hEsc ){
        hEsc=GetModuleHandle(L"esc.dll");
        if( !hEsc ){
            szText[0]=GetMsg(ModuleNumber,IDS_Rewrap);
            szText[1]=GetMsg(ModuleNumber,IDS_NoEsc);
            szText[2]=GetMsg(ModuleNumber,IDS_Cancel);
            Message(ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,szEsc,szText,3,1);
            return INVALID_HANDLE_VALUE;
        }
    }
    if( !GetEditorSettings ){
        GetEditorSettings=(int (WINAPI*)(int, const wchar_t*, void*))GetProcAddress(hEsc,"GetEditorSettingsW");
        if( !GetEditorSettings ){
            szText[0]=GetMsg(ModuleNumber,IDS_Rewrap);
            szText[1]=GetMsg(ModuleNumber,IDS_OldEsc);
            szText[2]=GetMsg(ModuleNumber,IDS_Cancel);
            Message(ModuleNumber,FMSG_ERRORTYPE|FMSG_WARNING,szEsc,szText,3,1);
            return INVALID_HANDLE_VALUE;
        }
    }

    EditorControl(ECTL_GETINFO,&ei);

    GetEditorSettings(ei.EditorID,L"wrap",&nWrapPos);
    GetEditorSettings(ei.EditorID,L"justify",&isJustifyEnabled);
    GetEditorSettings(ei.EditorID,L"p_quote",&isWrapQuote);

    if( nWrapPos<1 || nWrapPos>512 ){
        szText[0]=GetMsg(ModuleNumber,IDS_Rewrap);
        szText[1]=GetMsg(ModuleNumber,IDS_NoWrap);
        szText[2]=GetMsg(ModuleNumber,IDS_Cancel);
        Message(ModuleNumber,FMSG_WARNING,szEsc,szText,3,1);
        return INVALID_HANDLE_VALUE;
    }

    esp.CurPos=0;
    esp.CurTabPos=-1;
    esp.TopScreenLine=-1;
    esp.LeftPos=-1;
    esp.Overtype=-1;

    if( ei.BlockType!=BTYPE_NONE ){
        esp.CurLine=ei.BlockStartLine;
        EditorControl(ECTL_SETPOSITION,&esp);
    }

    nlsSpace=L' ';

    nIndent1=nIndent2=-1;
    pMem=NULL;
    nLen=0;
    isBlank=1;
    ess.StringLength=0;
    nAddLine=0;
    nQuote=0;
    szQuote=NULL;
    hHeap=GetProcessHeap();

    eur.Command=EUR_BEGIN;
    EditorControl(ECTL_UNDOREDO,&eur);

    do{

        i=-1;
        EditorControl(ECTL_EXPANDTABS,&i);

        egs.StringNumber=-1;
        EditorControl(ECTL_GETSTRING,&egs);

        if( !pMem ){
            if( isWrapQuote ){
                nQuote=IsQuote(egs.StringText,egs.StringLength);
                if( nQuote ){
                    if( nQuote>=(nWrapPos-1) )nQuote=0;
                    else{
                        szQuote=(wchar_t*)HeapAlloc(hHeap,HEAP_GENERATE_EXCEPTIONS,(nQuote+1)*sizeof(wchar_t));
                        wmemcpy(szQuote,egs.StringText,nQuote);
                        szQuote[nQuote]=L'\0';
                    }
                }
            }else nQuote=0;
        }

        if( nQuote==0 ){
            if( nIndent1==-1 ){
                for( nIndent1=0; nIndent1<egs.StringLength; nIndent1++ )
                    if( !IsCSpace(egs.StringText[nIndent1]) ) break;
                if( ei.BlockType==BTYPE_NONE && nIndent1==egs.StringLength ){
                    esp.CurLine=ei.CurLine+1;
                    esp.CurPos=0;
                    EditorControl(ECTL_SETPOSITION,&esp);
                    eur.Command=EUR_END;
                    EditorControl(ECTL_UNDOREDO,&eur);
                    return INVALID_HANDLE_VALUE;
                }
            }else if( nIndent2==-1 ){
                for( nIndent2=0; nIndent2<egs.StringLength; nIndent2++ )
                    if( !IsCSpace(egs.StringText[nIndent2]) ) break;
                if( ei.BlockType==BTYPE_NONE && nIndent2==egs.StringLength ){
                    nAddLine=1;
                    break;
                }
            }else if( ei.BlockType==BTYPE_NONE ){
                for( i=0; i<egs.StringLength; i++ )
                    if( !IsCSpace(egs.StringText[i]) ) break;
                if( i==egs.StringLength ){
                    nAddLine=1;
                    break;
                }
                if( i!=nIndent2 ) break;
            }
        }else{// there is a quote in the first line...
            i=IsQuote(egs.StringText,egs.StringLength);
            if( ei.BlockType==BTYPE_NONE && (!IsSameQuote(szQuote,nQuote,egs.StringText,i)) ){
                for( j=0; j<egs.StringLength; j++ )
                    if( !IsCSpace(egs.StringText[j]) ) break;
                if( j==egs.StringLength )nAddLine=1;
                break;
            }
            egs.StringText+=i;
            egs.StringLength-=i;
        }

        if( pMem ){
            pMem=(wchar_t*)HeapReAlloc(hHeap,HEAP_GENERATE_EXCEPTIONS,pMem,(nLen+egs.StringLength+1)*sizeof(wchar_t));
            if( !isBlank ){
                pMem[nLen++]=(char)nlsSpace;
                isBlank=1;
            }
        }else
            pMem=(wchar_t*)HeapAlloc(hHeap,HEAP_GENERATE_EXCEPTIONS,egs.StringLength*sizeof(wchar_t));

        for( i=0; i<egs.StringLength; i++ ){
            if( IsCSpace(egs.StringText[i]) ){
                if( isBlank ) continue;
                else isBlank=1;
            }else isBlank=0;
            pMem[nLen++]=egs.StringText[i];
        }

        EditorControl(ECTL_DELETESTRING,NULL);

        if( ei.BlockType!=BTYPE_NONE ){
            egs.StringNumber=-1;
            EditorControl(ECTL_GETSTRING,&egs);
        }

    }while( (egs.SelStart>=0 && egs.SelStart!=egs.SelEnd) ||
            ei.BlockType==BTYPE_NONE
          );

    if( nLen && IsCSpace(pMem[nLen-1]) ) nLen--;

    if( nLen==0 ){
        EditorControl(ECTL_INSERTSTRING,0);
        eur.Command=EUR_END;
        EditorControl(ECTL_UNDOREDO,&eur);
        return INVALID_HANDLE_VALUE;
    }

    if( nIndent1>=nWrapPos-1 || nIndent1<0 ) nIndent1=0;
    if( nIndent2>=nWrapPos-1 || nIndent2<0 ) nIndent2=nIndent1;

    ess.StringText=(wchar_t*)HeapAlloc(hHeap,HEAP_GENERATE_EXCEPTIONS,(nWrapPos+2)*sizeof(wchar_t));
    nStart=0;

    EditorControl(ECTL_GETINFO,&ei);

    while( nStart<nLen ){

        wmemset((wchar_t*)ess.StringText,nlsSpace,nIndent1);
        wmemcpy((wchar_t*)&ess.StringText[nIndent1],szQuote,nQuote);
        nPara=ess.StringLength=j=nIndent1+nQuote;
        nIndent1=nIndent2;

        for( i=nStart; i<nLen; i++ ){
            ((wchar_t*)ess.StringText)[j]=pMem[i];
            if( IsCSpace(pMem[i]) ){
                ess.StringLength=j++;
                nStart=i+1;
            }else{
                if( ++j>nWrapPos ){
                    i--;
                    break;
                }
            }
        }
        if( i==nLen ){
            nStart=i;
            ess.StringLength=j;
        }else if( i<nLen ){
            if( nIndent1==ess.StringLength ){
                nIndent1=0;
                ess.StringLength=j-1;
                nStart=i+1;
            }else if( isJustifyEnabled && ess.StringLength<nWrapPos ){
                for( j=0, i=nPara; i<ess.StringLength; i++ )
                    if( IsCSpace(ess.StringText[i]) )j++;
                // j==amount of meaning blanks==word count-1
                if( j ){
                    SpaceCount.quot = nWrapPos-ess.StringLength+j;
                    SpaceCount.rem = SpaceCount.quot % j;
                    SpaceCount.quot = SpaceCount.quot / j;
                    // now we have minimum space length in SpaceCount.quot
                    // and amount of blank fields with extra spacing in SpaceCount.rem
                    for( j=nWrapPos-1, i=ess.StringLength-1; i>=nPara; i-- ){
                        if( IsCSpace(ess.StringText[i]) ){
                            if( SpaceCount.rem ){
                                SpaceCount.rem--;
                                ((wchar_t*)ess.StringText)[j--]=nlsSpace;
                            }
                            j-=SpaceCount.quot;
                            wmemset( (wchar_t*)ess.StringText+j+1, nlsSpace, SpaceCount.quot );
                        }else
                            ((wchar_t*)ess.StringText)[j--]=ess.StringText[i];
                    }
                    ess.StringLength=nWrapPos;
                }
            }
        }

        if( ei.CurLine==-1 ){
            egs.StringNumber=-1;
            EditorControl(ECTL_GETSTRING,&egs);
            esp.CurLine=-1;
            esp.CurPos=egs.StringLength;
            EditorControl(ECTL_SETPOSITION,&esp);
        }
        EditorControl(ECTL_INSERTSTRING,0);
        if( ei.CurLine!=-1 ){
            esp.CurLine=ei.CurLine;
            EditorControl(ECTL_SETPOSITION,&esp);
            ei.CurLine=-1;
        }

        ess.StringNumber=-1;
        ess.StringEOL=egs.StringEOL;
        EditorControl(ECTL_SETSTRING,&ess);

    }

    HeapFree(hHeap,0,(wchar_t*)ess.StringText);
    HeapFree(hHeap,0,szQuote);
    HeapFree(hHeap,0,pMem);

    es.BlockType=BTYPE_NONE;
    EditorControl(ECTL_SELECT,&es);

    EditorControl(ECTL_GETINFO,&ei);

    esp.CurLine=ei.CurLine+1+nAddLine;
    esp.CurPos=0;
    EditorControl(ECTL_SETPOSITION,&esp);

    eur.Command=EUR_END;
    EditorControl(ECTL_UNDOREDO,&eur);

    EditorControl(ECTL_REDRAW,NULL);

    return INVALID_HANDLE_VALUE;
}

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
#endif
