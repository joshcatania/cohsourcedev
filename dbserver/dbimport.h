#ifndef DBIMPORT_H
#define DBIMPORT_H

C_DECLARATIONS_BEGIN

// This is for importing a dump file in to the database

void importDump(int argc,char **argv);
void fixImport(int argc,char **argv);
void exportRawDump(int argc, char **argv);

C_DECLARATIONS_END

#endif