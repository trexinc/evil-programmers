def GetGlobalInfoW(info):
    info["Title"] = "Hello Python world"
    info["Author"] = "Far group"
    info["Description"] = "Python plugin very basic example"
    info["Guid"] = "31A0D11E-B9D8-4A9B-88C7-2D2983802C51"

def GetPluginInfoW(info):
    info["MenuString"] = "Hello Python world"
    info["Guid"] = "DAF1257B-E011-4B5A-B5DC-732581BDF3BA";

def OpenW(info):
    print("Hello Python world\n")
