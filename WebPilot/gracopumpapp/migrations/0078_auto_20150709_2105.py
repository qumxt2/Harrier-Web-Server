# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


def add_new_tos_placeholder(apps, schema_editor):
    '''Add a new placeholder terms-of-service to test what happens when the TOS is updated'''
    TermsOfService = apps.get_model("gracopumpapp", 'TermsOfService')
    content = '''
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque a tempus ipsum. Vivamus blandit auctor ante. Mauris ac bibendum est. Cras in viverra risus. Phasellus varius risus nec sapien ullamcorper fermentum. Fusce ut nisi vitae ligula sodales convallis in in velit. Nulla facilisi. Fusce mattis, nibh a feugiat lacinia, libero enim vestibulum ligula, at imperdiet sem ex at turpis. Curabitur condimentum neque arcu, quis convallis nulla pellentesque at. Duis pulvinar posuere luctus. Aliquam nec maximus nulla. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Suspendisse semper enim quis lectus posuere, sit amet tristique nulla mattis.

Cras sim turpis, feugiat sed nunc id, vestibulum suscipit justo. Aenean vitae vehicula purus, eu luctus neque. Nam vel nunc et est facilisis ultrices id vel ante. Ut lacinia sapien leo, at vestibulum orci porta ut. In vehicula lectus tristique eleifend hendrerit. Suspendisse vehicula placerat est, eu ullamcorper lorem ullamcorper consequat. Vivamus sodales erat eu nibh sagittis, aliquet hendrerit orci sagittis. Nam pellentesque, odio vel pretium eleifend, velit eros dignissim dui, interdum ultricies metus lacus at tortor.

Mauris sodales id erat at sollicitudin. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Maecenas eget ante at sem pellentesque tincidunt eget non elit. Aenean laoreet felis id mollis posuere. Aenean hendrerit purus eu porttitor malesuada. Sed urna nibh, luctus in nibh nec, sagittis tincidunt neque. Vestibulum ligula tortor, porta non accumsan et, sagittis nec libero. Etiam rhoncus mauris mattis pretium consequat. Praesent posuere sed odio nec pellentesque. Maecenas consequat libero magna, accumsan sodales lorem rutrum at. In pellentesque metus in augue vulputate ultrices. Donec elit tortor, viverra a mauris et, aliquam aliquet leo. Curabitur vitae nulla quis eros rutrum tincidunt.

Suspendisse in sagittis leo. Aliquam erat volutpat. Integer ac mi eu magna facilisis lobortis in nec arcu. Ut lacinia ante elit, et elementum diam condimentum et. Donec ultricies varius erat non imperdiet. Aliquam semper, velit in eleifend scelerisque, tortor tortor tempus justo, in dictum tellus mauris ultrices sapien. Fusce lectus ante, laoreet quis malesuada sed, imperdiet non tellus. Aliquam pretium, massa eget bibendum aliquet, eros turpis volutpat augue, ac luctus tellus dolor vitae quam. Nam sit amet imperdiet massa, ac tristique felis. Cras eu feugiat libero, id sodales nunc. Nullam eleifend posuere risus quis lobortis.

Nulla ullamcorper luctus elementum. Cras pharetra massa lorem, sit amet elementum erat tincidunt id. Quisque enim erat, suscipit ac aliquam et, porta a urna. Praesent auctor odio euismod ligula sagittis fringilla. Etiam semper auctor interdum. Quisque semper placerat porttitor. Nulla fermentum, augue non pulvinar accumsan, risus tortor tempus orci, ut vehicula turpis felis non mi.

v2
'''
    TermsOfService.objects.create(content=content)


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0077_auto_20150707_1801'),
    ]

    operations = [
         migrations.RunPython(add_new_tos_placeholder)
    ]
