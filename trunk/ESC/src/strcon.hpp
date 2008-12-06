#ifndef __strcon_hpp
#define __strcon_hpp

class strcon
{
   public:
     strcon(const char *ptr, int size=-1);
     strcon(const unsigned char *ptr, int size=-1);
     strcon(const strcon &ptr);
     strcon();
     ~strcon();

   public:
     bool operator==(const strcon &ptr) const;
     bool operator==(const char * ptr) const;
     strcon& operator=(const char *ptr);
     strcon& operator=(const strcon &ptr);
     const strcon& AddStr(const char *s, int size=-1);
     const strcon& AddStr(const strcon &s);
     const strcon& AddChar(char Chr);
     const strcon& AddStrings(const char *firstStr, ...);
     unsigned int getLength() const { return Len; }
     bool setLength(unsigned newLen);
     char *setStr(const char *s, int size=-1);

   private:
     void Free();

   public:
     char *str;

   private:
     unsigned int Len;
};

#endif //__strcon_hpp
