if EXIST ..\..\..\bin\APP_CHEM_PUMP.gsi del ..\..\..\bin\APP_CHEM_PUMP.gsi
mkimage32.exe -b gcareset ..\..\..\dist\default\production\APP_CHEM_PUMP.production.hex ..\..\..\bin\APP_CHEM_PUMP.gsi
