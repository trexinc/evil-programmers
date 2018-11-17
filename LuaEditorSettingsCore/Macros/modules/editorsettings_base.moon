import
  Sources
  UnixSources
  from require'editorsettings_helpers'
ChangeLogMask="*changelog*"
CMask="*.[ch],*.[ch]pp,*.ii,*.cc,*.cxx,*.hrh,*.re,*.inl"
MFourMask="*.m4"
FarFolder=(Compare,Mask,FileName)->if FileName\match[[\far\FarManager\]] then Compare Mask,FileName else false
WhiteSpaceColor=
  Flags: bit64.bor far.Flags.FCF_FG_4BIT,far.Flags.FCF_BG_4BIT
  ForegroundColor: 0x8
  BackgroundColor: 0x1
class CFarSources extends Sources
  Title: "Far C/C++ Sources"
  Type: (Compare,FileName)->FarFolder Compare,CMask,FileName
  TabSize: 4
  ExpandTabs: false
  ShowWhiteSpace: 2
  WhiteSpaceColor: WhiteSpaceColor
  Weight: 100
class CSources extends Sources
  Title: "C/C++ Sources"
  Type: CMask
  ExpandTabs: 1
class PasSources extends Sources
  Title: "Pascal Sources"
  Type: "*.pas,*.dpr,*.pp,*.lpr"
  ExpandTabs: 1
class RustSources extends UnixSources
  Title: "Rust Sources"
  Type: "*.rs"
  ExpandTabs: 1
class LuaSources extends Sources
  Title: "Lua & MoonScript Sources"
  Type: "*.lua,*.moon"
  ExpandTabs: 1
class Io extends Sources
  Title: "Io Sources"
  Type: "*.io"
  ExpandTabs: 1
  Highlite:require'highlite.io'
class Makefile extends UnixSources
  Title: "makefile"
  Type: "*makefile*,*.mak"
  Highlite:require'highlite.make'
  TabSize: 8
  ExpandTabs: false
class MFourFar extends Sources
  Title: "Far m4"
  Type: (Compare,FileName)->FarFolder Compare,MFourMask,FileName
  Highlite:require'highlite.m4'
  Weight: 100
class MFour extends UnixSources
  Title: "m4"
  Type: MFourMask
  Highlite:require'highlite.m4'
class Bash extends UnixSources
  Title: "bash"
  Type: "*.sh"
  First:{"#![%s]*/bin/[%l]-sh","#![%s]*/usr/bin/env[%s]+[%l]-sh"}
  Highlite:require'highlite.bash'
class LexAndYacc extends UnixSources
  Title: "lex/yacc"
  Type: "*.l,*.y"
class Syntax extends UnixSources
  Title: "syntax"
  Type: "*.syntax,syntax"
  TabSize: 4
class Sql extends Sources
  Title: "sql"
  Type: "*.sql"
class JavaScript extends Sources
  Title: "js"
  Type: "*.js"
class VBScript extends Sources
  Title: "vbs"
  Type: "*.vbs"
  Highlite:require'highlite.vbs'
class FarMail extends Sources
  Title: "farmail script"
  Type: "*.fms"
class BatFile extends Sources
  Title: "bat file"
  Type: "*.bat,*.cmd"
class RegFile extends Sources
  Title: "reg file"
  Type: "*.reg"
  TabSize: 4
  Eol: "\r\n"
class Diff
  Title: "diff output"
  Type: "*.diff,*.rej,*.patch"
  First: {"^diff %-%-git%s","Index:%s"}
  Highlite:require'highlite.diff'
class Resources
  Title: "Resource file"
  Type: "*.rc"
  Highlite:require'highlite.rc'
class Postscript
  Title: "PostScript"
  Type: "*.ps,*.eps,*ai"
  Highlite:require'highlite.ps'
class Binaries
  LockMode: true
  TabSize: 8
  ExpandTabs: false
  AutoIndent: false
  CharCodeBase: 2
class Executables extends Binaries
  Title: "Binary executable files"
  Type: "*.exe,*.com,*.dll,*.bin,*.lib,*.obj,*.bav,*.srp,*.srs,*.class,*.msi,*.vxd,*.vdd,*.fmp"
class Archives extends Binaries
  Title: "Archives"
  Type: "*.rar,*.arj,*.jar,*.zip,*.lzh,*.ha,*.r[0-9][0-9],*.a[0-9][0-9],*.wsz,*.tar,*.gz,*.bz2,*.tgz,*.j,*.ace,*.z"
class Pictures extends Binaries
  Title: "Pictures"
  Type: "*.ani,*.bmp,*.bw,*.cur,*.dcx,*.dib,*.emf,*.eps,*.fpx,*.gif,*.ico,*.iff,*.ilbm,*.jpe,*.jpeg,*.jif,*.jfif,*.jpg,*.kdc,*.lbm,*.mag,*.pmb,*.pcd,*.pcx,*.pgm,*.pix,*.png,*.ppm,*.psd,*.ras,*.rgb,*.rgba,*.rle,*.sgi,*.tga,*.tif,*.tiff"
class Video extends Binaries
  Title: "Video files"
  Type: "*.mpg,*.mpeg,*.mp4,*.avi,*.divx,*.asf,*.wm,*.wmv"
class Audio extends Binaries
  Title: "Audio files"
  Type: "*.cda,*.mp2,*.mp3,*.wma,*.mid,*.midi,*.rmi,*.wav,*.snd,*.au,*.aif,*.aifc,*.aiff"
class OtherBinaries extends Binaries
  Title: "Other binary files"
  Type: "*.chm,*.lnk,*.pif,*.ttf,*.fon"
class Texts extends Sources
  TabSize: 8
  CharCodeBase: 1
class Messages extends Texts
  Title: "Messages"
  Type: "*.eml,*.nws"
  CodePage: 20866
  MinusMinusSpace: true
class FarHelp extends Texts
  Title: "FAR Help file"
  Type: "*.hlf"
  Eol: "\r\n"
class NtEvents extends Texts
  Title: "nt events"
  Type: "[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].err.info,[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].war.info,[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].inf.info,[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].aus.info,[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].auf.info,[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].info"
  TabSize: 10
class PlainText extends Texts
  Title: "Text"
  Type: "*.txt"
  CodePage: 1200
  SetBOM: true
class History extends Texts
  Title: "history.txt"
  Type: "*history*"
class ChangeLogCore extends Texts
  Type: ChangeLogMask
  TabSize: 8
  Highlite:require'highlite.changelog'
class ChangeLogFar extends ChangeLogCore
  Title: "ChangeLog far"
  Type: (Compare,FileName)->FarFolder Compare,ChangeLogMask,FileName
  ExpandTabs: false
  ShowWhiteSpace: 2
  WhiteSpaceColor: WhiteSpaceColor
  Weight: 100
class ChangeLog extends ChangeLogCore
  Title: "ChangeLog"
class Roadmap extends Texts
  Title: "roadmap"
  Type: "*roadmap*"
  Eol: "\r\n"
class Xml extends Sources
  Title: "xml"
  Type: "*.xml,*.xsl"
  TabSize: 8
  ExpandTabs: false
class Html extends Xml
  Title: "html"
  Type: "*.htm,*.html,*.shtml,*.php,*.css"
  CodePage: 65001
  SetBOM: false
class Csv
  Title: "csv"
  Type: "*.csv"
  Highlite: require'highlite.csv'
class Default
  Title: "default"
  Type: "*"
  Weight: 0
{
  :CFarSources, :CSources, :PasSources, :RustSources, :LuaSources, :Io, :Sql
  :Makefile, :MFourFar, :MFour, :Bash, :LexAndYacc, :Syntax
  :JavaScript, :VBScript, :FarMail, :BatFile, :RegFile
  :Diff, :Resources, :Postscript
  :Executables, :Archives, :Pictures, :Video, :Audio, :OtherBinaries
  :Messages, :FarHelp, :NtEvents, :PlainText, :History, :ChangeLogFar, :ChangeLog, :Roadmap
  :Xml, :Html, :Csv
  :Default
}
