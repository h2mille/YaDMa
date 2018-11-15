#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
import nfc
import sqlite3

def get_nfc():
    context = nfc.init()
    pnd = nfc.open(context)
    if pnd is None:
        print('ERROR: Unable to open NFC device.')
        exit()

    if nfc.initiator_init(pnd) < 0:
        nfc.perror(pnd, "nfc_initiator_init")
        print('ERROR: Unable to init NFC device.')
        exit()

    nmMifare = nfc.modulation()
    nmMifare.nmt = nfc.NMT_ISO14443A
    nmMifare.nbr = nfc.NBR_106
    nt = nfc.target()
    ret = nfc.initiator_select_passive_target(pnd, nmMifare, 0, 0, nt)


    tab = []
    i=0
    while i<nt.nti.nai.szUidLen:
        tab.append(nt.nti.nai.abtUid[i])
        i +=1
    nfc.close(pnd)
    nfc.exit(context)
    return tab

tab = get_nfc()
i=-1
print(tab)
conn=sqlite3.connect('user_database..db')
c = conn.cursor()
format_tab = "{}".format("".join('{:02x}'.format(len(tab))))
format_tab = format_tab+"{}".format("".join('{:02x}'.format(x) for x in tab))
print (format_tab)
response=c.execute('SELECT * FROM users WHERE card = "{}";'.format( format_tab))   
card =response.fetchone()
if(card==None):
    print("la carte n'est pas encore enregistrée")
    name = raw_input("indiquez l'utilisateur:")
    start = raw_input("indiquez la date de début (dd/mm/yyyy):")
    stop = raw_input("indiquez la date de fin (dd/mm/yyyy):")
    permission = 255
    print("placez à nouveau la carte sur le lecteur")
    tab2 = get_nfc()
    if (tab2 == tab):
        from subprocess import call
	call('./set_auth')		
        print('''INSERT INTO users VALUES (?,?,?,?,?);''', (name,format_tab, start, stop, permission))
        c.execute("INSERT INTO users VALUES (?,?,?,?,?);", (name,format_tab, start, stop, permission))
        print('utilisateur ajouté')
    else:
        print('mauvaise carte')

else:
    print("carte déjà enregistrée:")
    print(card);

conn.commit()

conn.close()
