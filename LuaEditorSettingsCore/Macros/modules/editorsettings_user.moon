------------------------------------------------------------------------------
-- This file is the place where user-defined schemes are loaded. User files
-- are moonscript modules and loaded with a line like the following:
-- schemes\add_items require'<modulefile>'
-- where <modulefile> is a "moonscript import path" to your module (i.e.
-- replace path separators with dots and do not include file extension). Add
-- your import lines where the hints below indicate.
------------------------------------------------------------------------------

import
  Schemes
  from require'editorsettings_helpers'

schemes=Schemes!

-- Start of user modules import lines
schemes\add_items require'editorsettings.python'
schemes\add_items require'editorsettings.yaml'
-- End of user modules import lines

schemes.items
