def GetBuild():
    '''DO NOT EDIT BUILD NUMBER -- DONE AUTOMATICALLY BY BUILD SCRIPT'''
    build = 440
    return build


def GetMajor():
    major = 1
    return major


def GetMinor():
    minor = 0
    return minor


def GetVersion():
    major = GetMajor()
    minor = GetMinor()
    build = GetBuild()
    return '%i.%i.%i' % (major, minor, build)


