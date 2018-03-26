#!/bin/bash

echo -e "\n[*] Updating last revision of SVN...\n"
sleep 5
svn checkout http://proxmark3.googlecode.com/svn/trunk/ .
if (( $? == 0 )); then
	echo -e "\n[+] Successfuly updated !!"
else	
	echo -e "\n[-] Error occured while updating"
fi