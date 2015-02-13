# ====================================================================
# files;
# --------------------------------------------------------------------

xmlattribute.o: xmlattribute.c node.h
xmldata.o: xmldata.c node.h
xmledit.o: xmledit.c node.h types.h error.h number.h memory.h
xmlelement.o: xmlelement.c node.h
xmlfree.o: xmlfree.c node.h
xmlnode.o: xmlnode.c node.h types.h
xmlopen.o: xmlopen.c node.h types.h error.h 
xmlscan.o: xmlscan.c node.h types.h error.h number.h
xmlschema.o: xmlschema.c node.h format.h 
xmlselect.o: xmlselect.c node.h 
xmlvalue.o: xmlvalue.c node.h

# ====================================================================
# files;
# --------------------------------------------------------------------

