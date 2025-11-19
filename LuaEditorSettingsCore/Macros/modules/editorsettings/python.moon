import
  UnixSources
  from require'editorsettings_helpers'
class Python extends UnixSources
  Title: "python"
  Type: "*.py,*.pyw"
  First:{
    "#![%s]*/bin/python",
    "#![%s]*/usr/bin/env[%s]+python",
  }
  CodePage: 65001
  TabSize: 4
  ExpandTabs: true
  SmartHome: true
  SetBOM: true
{
  :Python
}
