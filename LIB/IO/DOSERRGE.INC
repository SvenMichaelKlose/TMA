;Needs DOSERRMN.INC

#if !_mdltiny
_text   segment para
#else
        .code
#endif

TXT_doserrors:
	db"$Fehler im Programm : Ungltige Funktionsnummer !",10,13
	db"Der Author wsste nicht, wie er Ihnen danken sollte wenn Sie ihm eine genaue",10,13
	db"Beschreibung der Situation in der der Fehler auftrat senden wrden.",10,13
	db"Entsprechende Adressen finden Sie in der Dokumentation des Programmes.",10,13
	dB"An die unregistrierten User: Ich stelle keine unliebsamen Fragen.$"
	db"Konnte Quelldatei nicht finden.$"
	db"Es wurde ein ungltiges Verzeichnis angegeben.$"
	db"Es sind zu viele Dateien ge”ffnet.$"
	db"Zugriff nicht erlaubt.$"
	db"Ungltiger, logischer Zugriffskanal.$"
	db"Memory Control Blocks sind zerst”rt.$"
	db"Nicht gengend Speicherplatz vorhanden um Programm zu starten.",10,13
	db"Sehen Sie in der Dokumentation nach um dieses Problem zu l”sen.$"
	db"Angeforderter Speicherbereich ist ungltig.$"
	db"Ungltige Benutzerumgebung.$"
	db"Ungltiges Dateiformat.$"
	db"Zugriffsart auf Datei ist nicht m”glich.$"
	db"Ungltige Daten fr Zugriff auf Ger„t.$"
	db"DOS-Interner Fehler. Beschreibung nicht m”glich.$"
	db"Laufwerksbezeichnung ist ungltig.$"
	db"Kann aktuelles Verzeichnis nicht l”schen.$"
	db"Ungltiger Versuch das Laufwerk zu wechseln.$"
	db"Konnte die angegebene Quelldatei nicht finden.$" ;"Keine weiteren Dateien vorhanden.$"
	db"Der angew„hlte Datentr„ger ist schreibgeschtzt.$"
	db"Fehler beim Ansprechen des Laufwerktreibers.$"
	db"Das angesprochene Laufwerk ist nicht betriebsbereit.$"
	db"Ungltiger Befehl beim Ansprechen des Laufwerks.$"
	db"Ein CRC-Prfsummenfehler trat beim Lesen auf.$"
	db"Diskzugriff mit Parameterblock mit falscher Gr”áe.$"
	db"Konnte Spur auf Datentr„ger nicht finden.$"
	db"Kein DOS-Diskette im angesprochenem Laufwerk.$"
	db"Konnte Sektor auf angesprochenem Laufwerk nicht finden.$"
	db"Es fehlt Papier im Drucker.$"
	db"Fehler beim Schreiben eines Datenblocks auf Datentr„ger.$"
	db"Fehler beim Lesen eines Datenblocks von Datentr„ger.$"
	db"Schwerer Fehler beim Zugriff auf Ger„t.$"
	db"Im Zusammenhang mit Filesharing wurden die Zugriffsrechte fr die Datei",10,13 
	db"verletzt.$"
	db"Die Sperrung der Datei bzw. Teilen der Datei wurde nicht beachtet.$"
	db"Es befindet sich die falsche Diskette im Laufwerk. Zugriff nicht m”glich.$"
	db"Zugriff auf FCB nicht m”glich.$"
	db"Eine weitere Bestimmung von Zugriffsrechten ist nicht m”glich.",10,13
	db"Der Puffer fr die Zugriffsrechte ist voll.$"
	db"DOS-Interner Fehler. Beschreibung nicht m”glich.$"
	db"Dateizugriff konnte nicht vollendet werden.$"
	db"Ein Netzwerkfehler ist aufgetreten.$"

