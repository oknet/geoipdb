#!/bin/sh

./ipconv 0 0 0 1 > ip_CNC.txt
./ipconv 0 0 0 2 > ip_CTC.txt
./ipconv 0 0 0 3 > ip_GWBN.txt
./ipconv 0 0 0 4 > ip_CERNET.txt
./ipconv 0 0 0 5 > ip_CMNET.txt
./ipconv 0 0 0 6 > ip_UNICOM.txt
./ipconv 0 0 0 7 > ip_CRTC.txt
./ipconv 0 0 0 CN > ip_CN.txt
./ipconv 0 0 0 US > ip_US.txt
./ipconv 0 0 0 CA > ip_CA.txt
./ipconv 0 0 0 KR > ip_KR.txt
./ipconv 0 0 0 JP > ip_JP.txt
./ipconv 0 0 0 101 > ip_HK.txt
./ipconv 0 0 0 102 > ip_TW.txt
./ipconv 0 0 0 MO > ip_MO.txt
./gentopo 0 1 1 0 1 > region.user
./gentopo 0 1 1 1 0 > acl_cnc
./gentopo 0 1 1 2 0 > acl_ctc
./gentopo 0 1 1 4 0 > acl_edu
./gentopo 0 1 1 5 0 > acl_cmnet
./gentopo 0 1 1 7 0 > acl_crtc
