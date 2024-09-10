def GetBuild():
    '''DO NOT EDIT BUILD NUMBER -- DONE AUTOMATICALLY BY BUILD SCRIPT'''
    build = 59
    return build


def GetMajor():
    major = 0
    return major


def GetMinor():
    minor = 1
    return minor


def GetVersion():
    major = GetMajor()
    minor = GetMinor()
    build = GetBuild()
    return '%i.%i.%i' % (major, minor, build)
