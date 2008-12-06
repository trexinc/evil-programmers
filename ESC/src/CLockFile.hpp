#ifndef __CLockFile_hpp
#define __CLockFile_hpp

class CLockFile
{
  friend class NODEDATA;
  public:
    CLockFile(const char *FileName):hFile(INVALID_HANDLE_VALUE)
    {
      On(FileName);
    }

    ~CLockFile() { Off(); }

    void On(const char *FileName)
    {
      Off();
      Name=FileName;
      hFile=(FileName && *FileName)?
        CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,
          OPEN_EXISTING,0,NULL):INVALID_HANDLE_VALUE;
    }

    void Off()
    {
      if(IsOn())
      {
        CloseHandle(hFile);
        hFile=INVALID_HANDLE_VALUE;
      }
    }

    bool IsOn() const { return hFile!=INVALID_HANDLE_VALUE; }
    void On() { if(!IsOn()) On(Name.str); }

    CLockFile& operator=(const CLockFile &rhs)
    {
      if(this!=&rhs)
      {
        Off();
        On(rhs.Name.str);
      }
      return *this;
    }

  private:
    HANDLE hFile;
    strcon Name;

  private:
    CLockFile(const CLockFile& rhs);

};


#endif // __CLockFile_hpp
