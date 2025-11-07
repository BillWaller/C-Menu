install:
./inst.sh --
./inst.sh menu $(PREFIX)/bin menu 0711 $(OWNER) $(GROUP)
./inst.sh form $(PREFIX)/bin form 0711 $(OWNER) $(GROUP)
./inst.sh pick $(PREFIX)/bin pick 0711 $(OWNER) $(GROUP)
./inst.sh ckeys $(PREFIX)/bin ckeys 0711 $(OWNER) $(GROUP)
./inst.sh view $(PREFIX)/bin view 0711 $(OWNER) $(GROUP)
./inst.sh whence $(PREFIX)/bin whence 0711 $(OWNER) $(GROUP)
./inst.sh rsh $(PREFIX)/bin rsh 4711 root root
./inst.sh enterchr $(PREFIX)/bin enterchr 0711 $(OWNER) $(GROUP)
./inst.sh enterstr $(PREFIX)/bin enterstr 0711 $(OWNER) $(GROUP)
./inst.sh iloan $(PREFIX)/bin iloan 0711 $(OWNER) $(GROUP)
./inst.sh -l
