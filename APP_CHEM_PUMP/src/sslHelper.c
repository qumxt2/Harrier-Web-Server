// sslHelper.c

// Copyright 2015
// Graco, Inc., Minneapolis, MN
// All Rights Reserved

// Chemical Pump Controller
// SSL/TLS communications. Not to be confused with ssl.c, which is part of WolfSSL

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "rtos.h"
#include "stdio.h"
#include "sslHelper.h"
#include "debug_app.h"
#include "modemTask.h"
#include "socketModem.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/memory.h"
#include "utilities.h"

// ******************************************************************************************
// CONSTANTS AND MACROS
// ******************************************************************************************


// ******************************************************************************************
// PUBLIC VARIABLES
// ******************************************************************************************



// ******************************************************************************************
// PRIVATE VARIABLES
// ******************************************************************************************
uint8 gSslResID = RTOS_INVALID_ID;

// SSL state variables
static WOLFSSL_METHOD *gSslMethod = NULL;
static WOLFSSL *gSslConn = NULL;
static WOLFSSL_CTX *gSslCtx = NULL;

// ******************************************************************************************
// PRIVATE FUNCTION PROTOTYPES
// ******************************************************************************************

static int sslRecvInterface(WOLFSSL *_ssl, char *buf, int sz, void *_ctx);
static int sslSendInterface(WOLFSSL *_ssl, char *buf, int sz, void *_ctx);

// ******************************************************************************************
// TRUSTED CAs
// ******************************************************************************************

// Additional CA certificates can be appended if they are also in PEM format
const unsigned char TrustedCAs[] = "-----BEGIN CERTIFICATE-----\n\
MIIGCDCCA/CgAwIBAgIQKy5u6tl1NmwUim7bo3yMBzANBgkqhkiG9w0BAQwFADCB\n\
hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n\
A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n\
BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQwMjEy\n\
MDAwMDAwWhcNMjkwMjExMjM1OTU5WjCBkDELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n\
EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n\
Q09NT0RPIENBIExpbWl0ZWQxNjA0BgNVBAMTLUNPTU9ETyBSU0EgRG9tYWluIFZh\n\
bGlkYXRpb24gU2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n\
ADCCAQoCggEBAI7CAhnhoFmk6zg1jSz9AdDTScBkxwtiBUUWOqigwAwCfx3M28Sh\n\
bXcDow+G+eMGnD4LgYqbSRutA776S9uMIO3Vzl5ljj4Nr0zCsLdFXlIvNN5IJGS0\n\
Qa4Al/e+Z96e0HqnU4A7fK31llVvl0cKfIWLIpeNs4TgllfQcBhglo/uLQeTnaG6\n\
ytHNe+nEKpooIZFNb5JPJaXyejXdJtxGpdCsWTWM/06RQ1A/WZMebFEh7lgUq/51\n\
UHg+TLAchhP6a5i84DuUHoVS3AOTJBhuyydRReZw3iVDpA3hSqXttn7IzW3uLh0n\n\
c13cRTCAquOyQQuvvUSH2rnlG51/ruWFgqUCAwEAAaOCAWUwggFhMB8GA1UdIwQY\n\
MBaAFLuvfgI9+qbxPISOre44mOzZMjLUMB0GA1UdDgQWBBSQr2o6lFoL2JDqElZz\n\
30O0Oija5zAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNV\n\
HSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwGwYDVR0gBBQwEjAGBgRVHSAAMAgG\n\
BmeBDAECATBMBgNVHR8ERTBDMEGgP6A9hjtodHRwOi8vY3JsLmNvbW9kb2NhLmNv\n\
bS9DT01PRE9SU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDBxBggrBgEFBQcB\n\
AQRlMGMwOwYIKwYBBQUHMAKGL2h0dHA6Ly9jcnQuY29tb2RvY2EuY29tL0NPTU9E\n\
T1JTQUFkZFRydXN0Q0EuY3J0MCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5jb21v\n\
ZG9jYS5jb20wDQYJKoZIhvcNAQEMBQADggIBAE4rdk+SHGI2ibp3wScF9BzWRJ2p\n\
mj6q1WZmAT7qSeaiNbz69t2Vjpk1mA42GHWx3d1Qcnyu3HeIzg/3kCDKo2cuH1Z/\n\
e+FE6kKVxF0NAVBGFfKBiVlsit2M8RKhjTpCipj4SzR7JzsItG8kO3KdY3RYPBps\n\
P0/HEZrIqPW1N+8QRcZs2eBelSaz662jue5/DJpmNXMyYE7l3YphLG5SEXdoltMY\n\
dVEVABt0iN3hxzgEQyjpFv3ZBdRdRydg1vs4O2xyopT4Qhrf7W8GjEXCBgCq5Ojc\n\
2bXhc3js9iPc0d1sjhqPpepUfJa3w/5Vjo1JXvxku88+vZbrac2/4EjxYoIQ5QxG\n\
V/Iz2tDIY+3GH5QFlkoakdH368+PUq4NCNk+qKBR6cGHdNXJ93SrLlP7u3r7l+L4\n\
HyaPs9Kg4DdbKDsx5Q5XLVq4rXmsXiBmGqW5prU5wfWYQ//u+aen/e7KJD2AFsQX\n\
j4rBYKEMrltDR5FL1ZoXX/nUh8HCjLfn4g8wGTeGrODcQgPmlKidrv0PJFGUzpII\n\
0fxQ8ANAe4hZ7Q7drNJ3gjTcBpUC2JD5Leo31Rpg0Gcg19hCC0Wvgmje3WYkN5Ap\n\
lBlGGSW4gNfL1IYoakRwJiNiqZ+Gb7+6kHDSVneFeO/qJakXzlByjAA6quPbYzSf\n\
+AZxAeKCINT+b72x\n\
-----END CERTIFICATE-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIF2DCCA8CgAwIBAgIQTKr5yttjb+Af907YWwOGnTANBgkqhkiG9w0BAQwFADCB\n\
hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n\
A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n\
BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAwMTE5\n\
MDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n\
EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n\
Q09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNh\n\
dGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCR\n\
6FSS0gpWsawNJN3Fz0RndJkrN6N9I3AAcbxT38T6KhKPS38QVr2fcHK3YX/JSw8X\n\
pz3jsARh7v8Rl8f0hj4K+j5c+ZPmNHrZFGvnnLOFoIJ6dq9xkNfs/Q36nGz637CC\n\
9BR++b7Epi9Pf5l/tfxnQ3K9DADWietrLNPtj5gcFKt+5eNu/Nio5JIk2kNrYrhV\n\
/erBvGy2i/MOjZrkm2xpmfh4SDBF1a3hDTxFYPwyllEnvGfDyi62a+pGx8cgoLEf\n\
Zd5ICLqkTqnyg0Y3hOvozIFIQ2dOciqbXL1MGyiKXCJ7tKuY2e7gUYPDCUZObT6Z\n\
+pUX2nwzV0E8jVHtC7ZcryxjGt9XyD+86V3Em69FmeKjWiS0uqlWPc9vqv9JWL7w\n\
qP/0uK3pN/u6uPQLOvnoQ0IeidiEyxPx2bvhiWC4jChWrBQdnArncevPDt09qZah\n\
SL0896+1DSJMwBGB7FY79tOi4lu3sgQiUpWAk2nojkxl8ZEDLXB0AuqLZxUpaVIC\n\
u9ffUGpVRr+goyhhf3DQw6KqLCGqR84onAZFdr+CGCe01a60y1Dma/RMhnEw6abf\n\
Fobg2P9A3fvQQoh/ozM6LlweQRGBY84YcWsr7KaKtzFcOmpH4MN5WdYgGq/yapiq\n\
crxXStJLnbsQ/LBMQeXtHT1eKJ2czL+zUdqnR+WEUwIDAQABo0IwQDAdBgNVHQ4E\n\
FgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB\n\
/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAArx1UaEt65Ru2yyTUEUAJNMnMvl\n\
wFTPoCWOAvn9sKIN9SCYPBMtrFaisNZ+EZLpLrqeLppysb0ZRGxhNaKatBYSaVqM\n\
4dc+pBroLwP0rmEdEBsqpIt6xf4FpuHA1sj+nq6PK7o9mfjYcwlYRm6mnPTXJ9OV\n\
2jeDchzTc+CiR5kDOF3VSXkAKRzH7JsgHAckaVd4sjn8OoSgtZx8jb8uk2Intzna\n\
FxiuvTwJaP+EmzzV1gsD41eeFPfR60/IvYcjt7ZJQ3mFXLrrkguhxuhoqEwWsRqZ\n\
CuhTLJK7oQkYdQxlqHvLI7cawiiFwxv/0Cti76R7CZGYZ4wUAc1oBmpjIXUDgIiK\n\
boHGhfKppC3n9KUkEEeDys30jXlYsQab5xoq2Z0B15R97QNKyvDb6KkBPvVWmcke\n\
jkk9u+UJueBPSZI9FoJAzMxZxuY67RIuaTxslbH9qh17f4a+Hg4yRvv7E491f0yL\n\
S0Zj/gA0QHDBw7mh3aZw4gSzQbzpgJHqZJx64SIDqZxubw5lT2yHh17zbqD5daWb\n\
QOhTsiedSrnAdyGN/4fy3ryM7xfft0kL0fJuMAsaDk527RH89elWsn2/x20Kk4yl\n\
0MC2Hb46TpSi125sC8KKfPog88Tk5c0NqMuRkrF8hey1FGlmDoLnzc7ILaZRfyHB\n\
NVOFBkpdn627G190\n\
-----END CERTIFICATE-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIENjCCAx6gAwIBAgIBATANBgkqhkiG9w0BAQUFADBvMQswCQYDVQQGEwJTRTEU\n\
MBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFkZFRydXN0IEV4dGVybmFs\n\
IFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBFeHRlcm5hbCBDQSBSb290\n\
MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFowbzELMAkGA1UEBhMCU0Ux\n\
FDASBgNVBAoTC0FkZFRydXN0IEFCMSYwJAYDVQQLEx1BZGRUcnVzdCBFeHRlcm5h\n\
bCBUVFAgTmV0d29yazEiMCAGA1UEAxMZQWRkVHJ1c3QgRXh0ZXJuYWwgQ0EgUm9v\n\
dDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALf3GjPm8gAELTngTlvt\n\
H7xsD821+iO2zt6bETOXpClMfZOfvUq8k+0DGuOPz+VtUFrWlymUWoCwSXrbLpX9\n\
uMq/NzgtHj6RQa1wVsfwTz/oMp50ysiQVOnGXw94nZpAPA6sYapeFI+eh6FqUNzX\n\
mk6vBbOmcZSccbNQYArHE504B4YCqOmoaSYYkKtMsE8jqzpPhNjfzp/haW+710LX\n\
a0Tkx63ubUFfclpxCDezeWWkWaCUN/cALw3CknLa0Dhy2xSoRcRdKn23tNbE7qzN\n\
E0S3ySvdQwAl+mG5aWpYIxG3pzOPVnVZ9c0p10a3CitlttNCbxWyuHv77+ldU9U0\n\
WicCAwEAAaOB3DCB2TAdBgNVHQ4EFgQUrb2YejS0Jvf6xCZU7wO94CTLVBowCwYD\n\
VR0PBAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wgZkGA1UdIwSBkTCBjoAUrb2YejS0\n\
Jvf6xCZU7wO94CTLVBqhc6RxMG8xCzAJBgNVBAYTAlNFMRQwEgYDVQQKEwtBZGRU\n\
cnVzdCBBQjEmMCQGA1UECxMdQWRkVHJ1c3QgRXh0ZXJuYWwgVFRQIE5ldHdvcmsx\n\
IjAgBgNVBAMTGUFkZFRydXN0IEV4dGVybmFsIENBIFJvb3SCAQEwDQYJKoZIhvcN\n\
AQEFBQADggEBALCb4IUlwtYj4g+WBpKdQZic2YR5gdkeWxQHIzZlj7DYd7usQWxH\n\
YINRsPkyPef89iYTx4AWpb9a/IfPeHmJIZriTAcKhjW88t5RxNKWt9x+Tu5w/Rw5\n\
6wwCURQtjr0W4MHfRnXnJK3s9EK0hZNwEGe6nQY1ShjTK3rMUUKhemPR5ruhxSvC\n\
Nr4TDea9Y355e6cJDUCrat2PisP29owaQgVR1EX1n6diIWgVIEM8med8vSTYqZEX\n\
c4g/VhsxOBi0cQ+azcgOno4uG+GMmIPLHzHxREzGBHNJdmAPx/i9F4BrLunMTA5a\n\
mnkPIAou1Z5jJh5VkpTYghdae9C8x49OhgQ=\n\
-----END CERTIFICATE-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIElDCCA3ygAwIBAgIQAf2j627KdciIQ4tyS8+8kTANBgkqhkiG9w0BAQsFADBh\n\
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n\
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n\
QTAeFw0xMzAzMDgxMjAwMDBaFw0yMzAzMDgxMjAwMDBaME0xCzAJBgNVBAYTAlVT\n\
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxJzAlBgNVBAMTHkRpZ2lDZXJ0IFNIQTIg\n\
U2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n\
ANyuWJBNwcQwFZA1W248ghX1LFy949v/cUP6ZCWA1O4Yok3wZtAKc24RmDYXZK83\n\
nf36QYSvx6+M/hpzTc8zl5CilodTgyu5pnVILR1WN3vaMTIa16yrBvSqXUu3R0bd\n\
KpPDkC55gIDvEwRqFDu1m5K+wgdlTvza/P96rtxcflUxDOg5B6TXvi/TC2rSsd9f\n\
/ld0Uzs1gN2ujkSYs58O09rg1/RrKatEp0tYhG2SS4HD2nOLEpdIkARFdRrdNzGX\n\
kujNVA075ME/OV4uuPNcfhCOhkEAjUVmR7ChZc6gqikJTvOX6+guqw9ypzAO+sf0\n\
/RR3w6RbKFfCs/mC/bdFWJsCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8C\n\
AQAwDgYDVR0PAQH/BAQDAgGGMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYY\n\
aHR0cDovL29jc3AuZGlnaWNlcnQuY29tMHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6\n\
Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwN6A1\n\
oDOGMWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RD\n\
QS5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8v\n\
d3d3LmRpZ2ljZXJ0LmNvbS9DUFMwHQYDVR0OBBYEFA+AYRyCMWHVLyjnjUY4tCzh\n\
xtniMB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA0GCSqGSIb3DQEB\n\
CwUAA4IBAQAjPt9L0jFCpbZ+QlwaRMxp0Wi0XUvgBCFsS+JtzLHgl4+mUwnNqipl\n\
5TlPHoOlblyYoiQm5vuh7ZPHLgLGTUq/sELfeNqzqPlt/yGFUzZgTHbO7Djc1lGA\n\
8MXW5dRNJ2Srm8c+cftIl7gzbckTB+6WohsYFfZcTEDts8Ls/3HB40f/1LkAtDdC\n\
2iDJ6m6K7hQGrn2iWZiIqBtvLfTyyRRfJs8sjX7tN8Cp1Tm5gr8ZDOo0rwAhaPit\n\
c+LJMto4JQtV05od8GiG7S5BNO98pVAdvzr508EIDObtHopYJeS4d60tbvVS3bR0\n\
j6tJLp07kzQoH3jOlOrHvdPJbRzeXDLz\n\
-----END CERTIFICATE-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n\
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n\
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n\
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n\
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n\
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n\
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n\
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n\
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n\
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n\
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n\
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n\
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n\
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n\
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n\
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n\
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n\
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n\
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n\
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n\
-----END CERTIFICATE-----\n\
";

// ******************************************************************************************
// PUBLIC FUNCTIONS
// ******************************************************************************************

// Initialize WolfSSL
bool SSL_Init(void)
{
    gSslResID = RtosResourceReserveID();
    (void)K_Resource_Get(gSslResID);

    gSslMethod = wolfTLSv1_2_client_method();

    gSslCtx = wolfSSL_CTX_new(gSslMethod);
    if (gSslCtx == NULL)
    {
        printf("SSL init failed\n");
        return FALSE;
    }

    // We want to validate the server's hostname against its certificate, or else
    // we shouldn't bother at all with any of this encryption
    wolfSSL_CTX_set_verify(gSslCtx, SSL_VERIFY_PEER, NULL);

    // Load the trusted CAs
    if (SSL_SUCCESS != wolfSSL_CTX_load_verify_buffer(gSslCtx, TrustedCAs, (long)sizeof(TrustedCAs), SSL_FILETYPE_PEM))
    {
        printf("SSL CA load failed\n");
        return FALSE;
    }

    wolfSSL_SetIORecv(gSslCtx, sslRecvInterface);
    wolfSSL_SetIOSend(gSslCtx, sslSendInterface);

    printf("SSL init succeeded\n");

    (void)K_Resource_Release(gSslResID);

    return TRUE;
}

// Using an already established socket connection, do the SSL handshake
//
// DOES NOT establish the socket connection in the modem
//
// The hostname is for validating the server's certificate
//
// Returns true if successful, false otherwise
bool SSL_Connect(const char* hostname, bool useSni)
{
    char errStr[81] = {0};
    sint32 errNum = 0;

    (void)K_Resource_Get(gSslResID);

    if (gSslConn)
    {
        SSL_Disconnect();
    }

    printf("SSL setup\n");
    
    // TODO: Might need to pump the RX queue here to make sure it's empty?

    // Ready the SSL connection. Won't actually try to connect until we try to
    // write data to the connection.
    gSslConn = wolfSSL_new(gSslCtx);
    
    if (!gSslConn)
    {
        errNum = wolfSSL_get_error(gSslConn, 0);
        wolfSSL_ERR_error_string_n(errNum, errStr, sizeof(errStr) - 1);
        printf("WolfSSL error: %s\n", errStr);

        return FALSE;
    }

    // Set up the domain name check. The actual check won't occur until inside
    // of wolfSSL_connect() via wolfSSL_write()
    (void)wolfSSL_check_domain_name(gSslConn, hostname);

    // Allow the use of SNI if, for example, we're hitting an HTTPS server
    if (useSni)
    {
        if (SSL_SUCCESS != wolfSSL_UseSNI(gSslConn, WOLFSSL_SNI_HOST_NAME, hostname, strlen(hostname)))
        {
            printf("SSL error setting up SNI\n");
            return FALSE;
        }
    }

    (void)K_Resource_Release(gSslResID);

    return TRUE;
}

// Shut down SSL
// DOES NOT disconnect the socket connection in the modem
void SSL_Disconnect(void)
{
    (void)K_Resource_Get(gSslResID);

    wolfSSL_free(gSslConn);
    gSslConn = NULL;
    printf("SSL connection closed\n");

    (void)K_Resource_Release(gSslResID);
}

// Read up to maxBytes from the SSL connection into buffer buf.
// Returns the number of bytes actually read.
int SSL_readBytes(uint8* buf, int maxBytes)
{
    sint16 bytesReadTotal = 0;
    sint16 bytesReadSub = 0;

    (void)K_Resource_Get(gSslResID);

    do
    {
        bytesReadSub = wolfSSL_read(gSslConn, (char*)buf, maxBytes-bytesReadTotal);
        if (bytesReadSub > 0)
        {
            bytesReadTotal += bytesReadSub;
        }
    } while (gSslConn != NULL &&
            wolfSSL_pending(gSslConn) &&
            bytesReadTotal < maxBytes &&
            bytesReadSub > 0);
    
    (void)K_Resource_Release(gSslResID);

    return bytesReadTotal;
}

// Tries to write numBytes bytes from buf out the SSL connection.
// Returns the number of bytes actually sent, or a negative number if an error.
sint16 SSL_writeBytes(const char* buf, uint16 numBytes)
{
    sint16 byteCount = 0;
    uint16 totalBytesWritten = 0;
    sint16 errorCode = -1;

    (void)K_Resource_Get(gSslResID);

    while (totalBytesWritten < numBytes)
    {
        byteCount = wolfSSL_write(gSslConn, (void*)&buf[totalBytesWritten], numBytes-totalBytesWritten);

        if (byteCount > 0)
        {
            totalBytesWritten += byteCount;
        }
        else
        {
            // We might still be setting up the connection, data might be unavailable, or we might
            // be in the midst of some mid-connection handshaking, but bail if there's a serious error.
            // Otherwise, stall here and keep retrying until we succeed or there's an actual error.
            errorCode = wolfSSL_get_error(gSslConn, 0);
            if (errorCode != SSL_ERROR_WANT_READ && errorCode != SSL_ERROR_WANT_WRITE)
            {
                printf("\nSSL write error: %d\n", errorCode);
                break;
            }
        }

        delay(100);
    }

    (void)K_Resource_Release(gSslResID);

    return byteCount;
}

// Returns true if WolfSSL is currently in a fatal error state
sint16 SSL_isError(void)
{
    sint16 errorCode = -1;

    (void)K_Resource_Get(gSslResID);

    if (gSslConn)
    {
        errorCode = wolfSSL_get_error(gSslConn, 0);
    }

    if (errorCode == SSL_ERROR_WANT_READ)
    {
        errorCode = SSL_ERROR_NONE;
    }

    (void)K_Resource_Release(gSslResID);
    
    return errorCode;
}


// ******************************************************************************************
// PRIVATE FUNCTIONS
// ******************************************************************************************

#if 0
// Make a test HTTPS connection. Intended to be removed once I've verified that WolfSSL is working.
static void sslTestHttps(void)
{
    char errStr[81] = {0};
    char sslBuf[256] = {0};
    sint32 errNum = 0;
    sint16 bytesRead = 0;
    sint16 bytesSent = 0;

    const char httpsGetRoot[] = "GET / HTTP/1.0\r\n\r\n";

    // Assume that a socket connection to graco-dev.graniteriver.com has been opened on port 443

    if (!SSL_Init())
    {
        return;
    }

    gSslConn = wolfSSL_new(gSslCtx);
    if (!gSslConn)
    {
        errNum = wolfSSL_get_error(gSslConn, 0);
        wolfSSL_ERR_error_string_n(errNum, errStr, sizeof(errStr) - 1);
        printf("WolfSSL error: %s\n", errStr);

        return;
    }
    else
    {
        printf("WolfSSL connection established.\n");
    }

    // Set up the domain name check. The actual check won't occur until inside
    // of wolfSSL_connect() via wolfSSL_write()
    wolfSSL_check_domain_name(gSslConn, HOST_FQDN);

    bytesSent = wolfSSL_write(gSslConn, httpsGetRoot, strlen(httpsGetRoot));
    printf("WolfSSL: %d bytes sent\n", bytesSent);

    delay(1000);

    do
    {
        bytesRead = wolfSSL_read(gSslConn, sslBuf, sizeof(sslBuf) - 1);
        if (bytesRead > 0)
        {
            sslBuf[bytesRead] = '\0';
            printf("WolfSSL rx (%d bytes): %s", bytesRead, sslBuf);
        }
    } while (wolfSSL_pending(gSslConn));

    wolfSSL_free(gSslConn);
    printf("WolfSSL: connection closed!\n");
}
#endif



// Interface for WolfSSL to receive encrypted data from the modem
// Returns the number of bytes read
static int sslRecvInterface(WOLFSSL *_ssl, char *buf, int sz, void *_ctx)
{
    // Add pause here (moved from in wolfSSLReceive() function in internal.c in old WolfSSL install)
    K_Task_Wait(1);
    
    int bytesReceived = 0;

    if (MODEM_Connected())
    {
        bytesReceived = getSocketModemData((uint8*)buf, sz);

        if (bytesReceived == 0)
        {
            bytesReceived = WOLFSSL_CBIO_ERR_WANT_READ;
        }
    }
    else
    {
        bytesReceived = WOLFSSL_CBIO_ERR_CONN_CLOSE;
    }

    return bytesReceived;
}

// Interface for WolfSSL to send data out of the modem
// Returns the number of bytes successfully sent. However, since we can't
// actually detect that, we'll just always assume the send was successful
// and return the number of bytes we were told to send.
static int sslSendInterface(WOLFSSL *_ssl, char *buf, int sz, void *_ctx)
{
    sint16 bytesSent = 0;

    if (MODEM_Connected())
    {
        bytesSent = MODEM_SendString((uint8*)buf, sz);
    }
    else
    {
        bytesSent = WOLFSSL_CBIO_ERR_CONN_CLOSE;
    }
    
    return bytesSent;
}

