from version import GetVersion


def get_version(request):
    '''For global version reporting'''
    software_version = GetVersion()
    return {'software_version': software_version}


def is_admin(request):
    '''For global admin control'''
    ret_val = False
    if request.user and request.user.is_authenticated():
        ret_val = request.user.userprofile.is_admin()

    return {'is_admin': ret_val}


def is_distributor(request):
    '''For global distributor control'''
    ret_val = False
    if request.user and request.user.is_authenticated():
        ret_val = request.user.userprofile.is_distributor()

    return {'is_distributor': ret_val}


def is_logged_in(request):
    ret_val = False
    if request.user and request.user.is_authenticated():
        ret_val = True

    return {'is_logged_in': ret_val}
