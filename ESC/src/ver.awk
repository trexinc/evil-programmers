# awk -f ver.awk> esc.rc
BEGIN {
  majorVer=2;
  minorVer=3;

  print '#define VERSIONINFO_1   1'
  print 'VERSIONINFO_1 VERSIONINFO'
  print
  printf ('FILEVERSION    %d,0,%d,0\n', majorVer, minorVer)
  printf ('PRODUCTVERSION %d,0,%d,0\n', majorVer, minorVer)
  print 'FILEFLAGSMASK  0x0'
  print 'FILEFLAGS      VS_FF_PRERELEASE'
  print 'FILEOS         VOS__WINDOWS32'
  print 'FILETYPE       VFT_DLL'
  print 'FILESUBTYPE    0x0'
  print '{'
  print '  BLOCK "StringFileInfo"'
  print '  {'
  print '    BLOCK "041904E3"'
  print '    {'
  print '      VALUE "CompanyName",       "SpinozaSoft\\0"'
  printf ('      VALUE "FileDescription",   "[ESC] Editor\'s settings changer v.%d.%d [%s] (FAR 1.7x PlugIn module)\\0"\n', majorVer, minorVer, strftime("%x %X"))
  printf ('      VALUE "FileVersion",       "%d.%d\\0"\n', majorVer, minorVer)
  print '      VALUE "Comments",          "Distribution: DonationWare and \\"AS IS\\", visit my web-site: http://www.moris.ru/~spinoza\\0"'
  print '      VALUE "InternalName",      "[ESC]\\0"'
  print '      VALUE "LegalCopyright",    "Ivan Sintyurin AKA Spinoza (spinoza@mail.ru)\\0"'
  print '      VALUE "LegalTrademarks",   "[ESC] is registered trademark of SpinozaSoft, IVS :-)\\0"'
  print '      VALUE "OriginalFilename",  "esc.dll\\0"'
  print '      VALUE "ProductName",       "Editor\'s settings changer\\0"'
  printf ('      VALUE "ProductVersion",    "%d.%d\\0"\n', majorVer, minorVer)
  print '    }'
  print '  }'
  print '  BLOCK "VarFileInfo"'
  print '  {'
  print '    VALUE "Translation", 0x419, 1251'
  print '  }'
  print '}'
}
