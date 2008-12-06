#ifndef __KeySequenceStorage_hpp
#define __KeySequenceStorage_hpp

#include "plugin.hpp"
#include "strcon.hpp"

enum KeySequenceStorageFlags
{
  KSSF_SELECTION_ON     = 0x01,
  KSSF_SELECTION_OFF    = 0x02,
  KSSF_SELECTION_STREAM = 0x04,
  KSSF_SELECTION_COLUMN = 0x08,
};

class KeySequenceStorage
{
  private:
    enum { KEY_MACRODATE = 0x00800001 };
    KeySequence Sequence;
    BOOL Stop;
    void Copy(const KeySequenceStorage& Value);
    void Init();
    inline int IsSpace(int x) { return x==0x20||x=='\t'||x=='\r'||x=='\n';}
  public:
    KeySequenceStorage(const char *str=NULL, bool silent=true, DWORD total=1, BOOL Stop=TRUE);
    KeySequenceStorage(const KeySequenceStorage& Value);
    ~KeySequenceStorage() { Free(); }
    const KeySequence& GetSequence(BOOL &stop) const
    {
      stop=Stop;
      return Sequence;
    }
    bool compile(const char *str, bool silent,DWORD total,BOOL stop,
                 int &Error, strcon &unknownKey);
    bool IsOK() const { return Sequence.Sequence; }
    void Free();
    KeySequenceStorage& operator=(const KeySequenceStorage& Value);
};

#endif // __KeySequenceStorage_hpp
