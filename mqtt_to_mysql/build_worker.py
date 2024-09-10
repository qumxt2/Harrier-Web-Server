"""
Build mqtt to db bridge worker
"""
import re
from subprocess import call, check_output


class BuildException(Exception):
    pass


def check_repo():
    '''Check for uncommitted work'''
    print('Checking for uncommitted work...\n')
    args = ['git', 'status', '.']
    status = check_output(args)
    if re.search(r'nothing to commit', status.decode()):
        return True
    else:
        print('*%s*' % status)
        return False


def commit_and_label(ver_string):
    print('Commit...\n')
    args = ['git', 'add', '.']
    if call(args) != 0:
        return False
    args = ['git', 'commit', '-m', 'Commit for mqtt-db worker build %s' % ver_string]
    if call(args) != 0:
        return False
    print('Tag as worker-%s...\n' % ver_string)
    args = ['git', 'tag', 'worker-%s' % ver_string]
    if call(args) != 0:
        return False
    args = ['git', 'push', ]
    if call(args) != 0:
        return False
    args = ['git', 'push', '--tags']
    if call(args) != 0:
        return False
    return True


def bump_version_number():
    # Bump the build number
    print('Bump version numbers...\n')
    version_file_sn = 'version.py'
    with open(version_file_sn, 'r+') as fd:
        content_sn = fd.read()
        ver_num_sn = int(re.search('(?<=build\s=\s)[0-9]+', content_sn).group(0))
        ver_num_sn += 1
        content_sn = re.sub('(?<=build\s=\s)[0-9]+', str(ver_num_sn), content_sn)
        ver_num_major_sn = int(re.search('(?<=major\s=\s)[0-9]+', content_sn).group(0))
        ver_num_minor_sn = int(re.search('(?<=minor\s=\s)[0-9]+', content_sn).group(0))
        ver_string = '%s.%s.%s' % (ver_num_major_sn, ver_num_minor_sn, str(ver_num_sn))

        fd.seek(0)
        fd.write(content_sn)

    return ver_string

if __name__ == '__main__':
    print('Graco mqtt to db bridge worker build script\n')

    try:
        if not check_repo():
            raise BuildException('Uncommitted changes in repo. Please commit first.')
        ver = bump_version_number()
        if not commit_and_label(ver):
            raise BuildException('Error committing and labeling!')

        print('Build successful. Done.\n')

    except BuildException as e:
        print('Error during build: %s' % e)
