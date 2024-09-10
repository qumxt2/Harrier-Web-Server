# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


def add_new_tos_version(apps, schema_editor):
    '''Add a new terms-of-service. This is the slightly-modified first "actual" TOS'''
    TermsOfService = apps.get_model("gracopumpapp", 'TermsOfService')
    content = '''End User License Agreement (EULA)

Graco Inc. agrees to provide you access to and use of its Software, under the terms and conditions specified below:


Configuration

You agree to use this Software with at least the minimum hardware and software requirements as set forth in the product documentation.


License Grant 

Graco grants you a license to use the Software. "Use" means storing, loading, installing, executing or displaying the Software. You may not modify the Software or disable any licensing or control features of the Software. You agree to use the Software only in conjunction with Graco hardware.


Ownership 

The Software is owned and copyrighted by Graco or its third party suppliers. Your license confers no title or ownership in the Software and is not a sale of any rights in the Software. Graco's third party suppliers may protect their rights in the event of any violation of these License Terms. 


Copyright 

The Software and any related documentation are protected by United States copyright law and international treaty provisions. You may not copy, modify, adapt, translate into any language, distribute, or create derivative works based on the Software without the prior written consent of Graco. You may not assign this EULA or any of the rights or licenses granted under this EULA or rent, lease, or lend the Software to any person or entity. Any attempted sublicense, transfer, or assignment in violation of this EULA is void. You acknowledge that the Software contains proprietary trade secrets of Graco and its suppliers. You agree not to decompile, disassemble, reverse engineer, or attempt to reconstruct, identify, or discover any source code, underlying user interface techniques, underlying ideas, or algorithms of the Software by any means whatsoever, except to the extent the foregoing restriction is prohibited by applicable law. Upon request, you will provide Graco with reasonably detailed information regarding any disassembly or decompilation. 


Privacy 

Graco Harrier is remote control and reporting technology for Graco’s chemical injection systems.  Graco Harrier is a subscription-based service.

If you consent when initially subscribing to Graco Harrier service at www.harrier.graco.com, Graco will collect certain personal information from you and collect data from your Graco chemical injection systems equipment.  Having access to personal information and to the equipment data is necessary in order to manage and operate Graco Harrier.
     
Personal information includes your name, company, telephone number and email address information.  Equipment data includes operating parameters, material usage reports, system warnings and alarms and location.
 
The information is collected for the purpose of providing trouble-shooting assistance and analysis, identifying the location of the equipment, conducting equipment use analyses, and other legitimate business purposes.

Graco will not share any of your personal information or equipment data with any other equipment end users, Graco distributors, material suppliers, trade organizations, homebuilders, or other third parties.   Graco will not sell any of your personal information or equipment data.
 
More specifically, Graco uses the equipment data at a macro level to design and build better products that are suited to meet customers’ needs.  Through analysis of the data, Graco tries to better understand how customers use Graco equipment.  Graco tries to understand, for example, how Graco equipment is used each day, the pressures, temperatures and flow rates at which users operate the equipment, etc.  Access to this data is limited and controlled by Graco.
 
In order for Graco technical support to view and access specific customer data, the Graco account owner must grant permission in their account settings.  The account owner can add or remove permissions at any time.

Graco employees may use the marked location of your equipment to help facilitate end user field visits, i.e., to identify the specific machine of interest.  Location is only available when the equipment is in operation. Graco may also use your contact information to keep you apprised of Graco news and any changes or updates to Graco Harrier.

Graco is a global company, and as such, recipients of your personal information and equipment data may be located in the United States or elsewhere.  Your personal information and equipment data will be held only as long as is necessary to accomplish the purposes described above.  You may at any time view your personal information and equipment data, request additional information about the storage and processing of your personal information and equipment data, require any necessary amendments to your personal information, or refuse or withdraw the consents given without cost by contacting Graco in writing.  However, refusing or withdrawing your consent may affect the ability of Graco to deliver certain services to you.
 

Termination 

Graco may terminate your license upon notice for failure to comply with any of these terms and conditions.

 
Export Requirements 

You may not use the Software in violation of any applicable laws or regulations. 


LIMITED WARRANTY STATEMENT 

a) Graco warrants that, when used with a recommended hardware and compatible software version configuration, the Software will perform in substantial conformance with the documentation supplied for the period of the warranty applying to the equipment with which it is used. EXCEPT AS SET FORTH IN THE FOREGOING LIMITED WARRANTY, GRACO DISCLAIMS ALL OTHER WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IF APPLICABLE LAW IMPLIES ANY WARRANTIES WITH RESPECT TO THE GRACO PRODUCT, ALL SUCH WARRANTIES ARE LIMITED IN DURATION TO NINETY (90) DAYS FROM THE DATE OF DELIVERY. No oral or written information or advice given by Graco, its dealers, distributors, agents or employees shall create a warranty or in any way increase the scope of this warranty. 

b) SOME STATES DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO THE ABOVE EXCLUSION MAY NOT APPLY TO YOU. THIS WARRANTY GIVES YOU SPECIFIC LEGAL RIGHTS AND YOU MAY ALSO HAVE OTHER LEGAL RIGHTS WHICH VARY FROM STATE TO STATE. 


Exclusive Remedy

Your exclusive remedy under this EULA is that Graco will use reasonable commercial efforts to modify the Software so that it substantially conforms to the documentation. Graco shall have no responsibility with respect to Software that has been altered in any way or if the nonconformance arises out of use of the Software in conjunction with software or hardware not supplied by Graco. 


Limitations of Damages

a) GRACO SHALL NOT BE LIABLE FOR ANY INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING DAMAGES FOR LOSS OF BUSINESS, LOSS OF PROFITS, OR THE LIKE), WHETHER BASED ON BREACH OF CONTRACT, TORT (INCLUDING NEGLIGENCE), PRODUCT LIABILITY OR OTHERWISE, EVEN IF GRACO OR ITS REPRESENTATIVES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES AND EVEN IF A REMEDY SET FORTH HEREIN IS FOUND TO HAVE FAILED OF ITS ESSENTIAL PURPOSE.
 
b) Graco's total liability to you for actual damages for any cause whatsoever will be limited to the amount paid by you for the Software that caused such damages 

c) SOME STATES DO NOT ALLOW THE LIMITATION OR EXCLUSION OF LIABILITY FOR INCIDENTAL OF CONSEQUENTIAL DAMAGES, SO THE ABOVE LIMITATION OR EXCLUSION MAY NOT APPLY TO YOU. 


Basis of Bargain

The limited warranty, exclusive remedies and limited liability set forth above are fundamental elements of the basis of the bargain between Graco and you. Graco would not be able to provide the Software on an economic basis without such limitations.


Copyright © 2015 Graco Inc. All rights reserved.

'''
    TermsOfService.objects.create(content=content)


class Migration(migrations.Migration):

    dependencies = [
        ('gracopumpapp', '0087_auto_20150724_0912'),
    ]

    operations = [
         migrations.RunPython(add_new_tos_version)
    ]
