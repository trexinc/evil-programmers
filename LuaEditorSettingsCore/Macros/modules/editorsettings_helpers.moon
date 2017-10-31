class Sources
  KillSpaces: true
  KillEmptyLines: true
  TabSize: 2
  AutoIndent: true
  CursorBeyondEol: true
  CharCodeBase: 2
  Lines: 64
  SmartHome: true
  SmartTab: true
  SmartBs: true
  Weight: 1
class UnixSources extends Sources
  Eol: "\n"

class Schemes
  empty_weight = 1
  __scheme_scales= (a,b) ->
    a_weight = a.Weight or empty_weight
    b_weight = b.Weight or empty_weight
    result = false
    if a_weight > b_weight
      result = true
    elseif a_weight == b_weight
      result = string.lower(a.Title) < string.lower(b.Title)
    result

  new: =>
    @items = {}

  add_items: (items) =>
    for k,v in pairs items
      @items[v.Title] = v

  get_schemes: =>
    values = {}
    for k,v in pairs @items
      table.insert values, v
    table.sort values, __scheme_scales
    values

{
  :Sources, :UnixSources
  :Schemes
}
