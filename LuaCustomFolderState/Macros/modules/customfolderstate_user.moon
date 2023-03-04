F=far.Flags
{
  [[%USERPROFILE%\Downloads\*]]
  {
    Name:[[dcim\camera]]
    PluginId:'9bf0e19d-b943-4e6d-b7b0-d71e3daf446a'
  }
  {
    Name:[[%USERPROFILE%\Documents]]
    --Action:(whatpanel,f)->(far.MacroPost [[Keys'F2']]) if 1==whatpanel
    Sort:false
  }
  Sort:F.SM_MTIME
  Order:true
}
