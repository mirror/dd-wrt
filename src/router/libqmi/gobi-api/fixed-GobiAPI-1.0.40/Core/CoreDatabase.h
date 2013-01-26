/*===========================================================================
FILE: 
   CoreDatabase.h

DESCRIPTION:
   Declaration of cCoreDatabase class

PUBLIC CLASSES AND METHODS:
   cCoreDatabase
      This class represents the run-time (read only) version of the 
      core library database

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include <list>
#include <map>
#include <set>
#include <vector>

#include "DB2TextFile.h"

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------
class cDB2NavTree;

//---------------------------------------------------------------------------
// Prototypes 
//---------------------------------------------------------------------------

// Convert a string (in quotes) to a string (minus) quotes and copy into 
// an allocated buffer
LPCSTR CopyQuotedString( LPSTR pString );

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// An empty (but not NULL) string
extern LPCSTR EMPTY_STRING;

// Value seperator for database text
extern LPCSTR DB2_VALUE_SEP;

// Sub-value (i.e. within a particular value) seperator for database text
extern LPCSTR DB2_SUBVAL_SEP;

// Database table file names
extern LPCSTR DB2_FILE_PROTOCOL_FIELD;
extern LPCSTR DB2_FILE_PROTOCOL_STRUCT;
extern LPCSTR DB2_FILE_PROTOCOL_ENTITY;
extern LPCSTR DB2_FILE_ENUM_MAIN;
extern LPCSTR DB2_FILE_ENUM_ENTRY;

// Database start pointers
extern const int _binary_QMI_Field_txt_start;
extern const int _binary_QMI_Struct_txt_start;
extern const int _binary_QMI_Entity_txt_start;
extern const int _binary_QMI_Enum_txt_start;
extern const int _binary_QMI_EnumEntry_txt_start;

// Database end pointers
extern const int _binary_QMI_Field_txt_end;
extern const int _binary_QMI_Struct_txt_end;
extern const int _binary_QMI_Entity_txt_end;
extern const int _binary_QMI_Enum_txt_end;
extern const int _binary_QMI_EnumEntry_txt_end;


// Status levels for DB2 logging
enum eDB2StatusLevel
{
   eDB2_STATUS_BEGIN = -1,

   eDB2_STATUS_INFO,         // Informational string
   eDB2_STATUS_WARNING,      // Warning string
   eDB2_STATUS_ERROR,        // Error string

   eDB2_STATUS_END 
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2StatusLevel validity check

PARAMETERS:
   lvl         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2StatusLevel lvl )
{
   bool retVal = false;
   if (lvl > eDB2_STATUS_BEGIN && lvl < eDB2_STATUS_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// Class cDB2StatusLog
//
//    Class that defines status logging interface for DB2 initialization
//    and exit related information
/*=========================================================================*/
class cDB2StatusLog
{
   public:
      // Log an error string
      virtual void Log( 
         LPCSTR                     pLog,
         eDB2StatusLevel            lvl = eDB2_STATUS_ERROR ) = 0;

      // Log an error string
      virtual void Log( 
         const std::string &        log,
         eDB2StatusLevel            lvl = eDB2_STATUS_ERROR ) = 0;
};

/*=========================================================================*/
// Class cDB2TraceLog
//    Default error logging interface for DB2 initialization and exit 
//    related information - sends output to TRACE
/*=========================================================================*/
class cDB2TraceLog : public cDB2StatusLog
{
   public:
      // (Inline) Constructor
      cDB2TraceLog() { };

      // (Inline) Destructor
      ~cDB2TraceLog() { };


      // (Inline) Log an error string
      virtual void Log( 
         LPCSTR                    pLog,
         eDB2StatusLevel           lvl = eDB2_STATUS_ERROR )
      {
         if (pLog != 0 && pLog[0] != 0)
         {
            std::string formatString = "[0x%02X] ";
            formatString += pLog;
            formatString += '\n';

            //Note: TRACE is just an alias for printf if DEBUG is used
            TRACE( formatString.c_str(), lvl );
         }
      };

      // (Inline) Log an error string
      virtual void Log( 
         const std::string &        log,
         eDB2StatusLevel            lvl = eDB2_STATUS_ERROR )         
      {
         if (log.size() > 0)
         {
            Log( log.c_str(), lvl );
         }
      };

};

// The default logger (for backwards compatibility)
extern cDB2TraceLog gDB2DefaultLog;


/*===========================================================================
METHOD:
   LoadDB2Table (Free Public Method)

DESCRIPTION:
   Load a database table
  
PARAMETERS:   
   pFile          [ I ] - The file to load the table from 
   cont           [I/0] - The current/resulting database table
   bDuplicatesOK  [ I ] - Duplicate keys acceptable?
   pName          [ I ] - Name (for error reporting)
   log            [I/O] - Where to log errors
   
RETURN VALUE:
   bool
===========================================================================*/
template <class Container>
bool LoadDB2Table( 
   LPCSTR                     pFile,            
   Container &                cont,
   bool                       bDuplicatesOK = false,
   LPCSTR                     pName = 0,
   cDB2StatusLog &            log = gDB2DefaultLog )
{
   // Assume success
   bool bRC = true;
   // Sanity check error reporting name
   if (pName == 0 || pName[0] == 0)
   {
      pName = "?";
   }

   // Sanity check file name
   if (pFile == 0 || pFile[0] == 0)
   {
      // Bad file
      std::ostringstream tmp;
      tmp << "DB [" << pName << "] Invalid file name";
      
      
      log.Log( tmp.str(), eDB2_STATUS_ERROR );

      return false;
   }

   ULONG lineNum = 0;

   // Attempt to open the file
   cDB2TextFile inFile( pFile );
   if (inFile.IsValid() == true)
   {
      std::string line;
      while (inFile.ReadLine( line ) == true)
      {
         std::string lineCopy = line;
         
         int nLineSize = lineCopy.size();
         LPSTR pLine = new CHAR[ nLineSize + 1 ];
         if (pLine == NULL)
         {
            return false;
         }

         memcpy( pLine, line.c_str(), nLineSize );

         // Enforce null terminator
         pLine[ nLineSize ] = 0;

         typename Container::mapped_type theType;
         bool bOK = theType.FromString( pLine );
         if (bOK == true)
         {
            // Grab key
            typename Container::key_type theKey = theType.GetKey();

            // Key already exists?
            typename Container::iterator pIter;
            pIter = cont.find( theKey );

            if (pIter != cont.end() && bDuplicatesOK == false)
            {
               // The key already exists which indicates a (recoverable) error
               std::ostringstream tmp;
               tmp << "DB [" << pName << "] Duplicate key, line "
                   << lineNum << " (" << line.c_str() << ")";
               
               log.Log( tmp.str(), eDB2_STATUS_WARNING );

               // Free the current object
               pIter->second.FreeAllocatedStrings();

               // ... and then replace it
               pIter->second = theType;
            }
            else
            {
               typename Container::value_type entry( theKey, theType );
               cont.insert( entry );
            }
         }
         else if (lineCopy.size() > 0)
         {
            // Error parsing line
            std::ostringstream tmp;
            tmp << "DB [" << pName << "] Parsing error, line "
                << lineNum << " (" << line.c_str() << ")";

            log.Log( tmp.str(), eDB2_STATUS_ERROR );

            theType.FreeAllocatedStrings();
            bRC = false;
         }
         delete [] pLine;
         lineNum++;
      }
   }
   else
   {
#ifdef DEBUG
      // Could not open the file
      std::ostringstream tmp;
      tmp << "DB [" << pName << "] Error opening file";

      log.Log( tmp.str(), eDB2_STATUS_WARNING );
#endif

      bRC = false;
   }

   return bRC;
};

/*===========================================================================
METHOD:
   LoadDB2Table (Free Public Method)

DESCRIPTION:
   Load a database table
  
PARAMETERS:   
   pStart         [ I ] - Start location of database
   nSize          [ I ] - Size of database
   cont           [I/0] - The current/resulting database table
   bDuplicatesOK  [ I ] - Duplicate keys acceptable?
   pName          [ I ] - Name (for error reporting)
   log            [I/O] - Where to log errors
   
RETURN VALUE:
   bool
===========================================================================*/
template <class Container>
bool LoadDB2Table( 
   const char *               pStart,
   const int                  nSize,
   Container &                cont,
   bool                       bDuplicatesOK = false,
   LPCSTR                     pName = 0,
   cDB2StatusLog &            log = gDB2DefaultLog )
{
   // Assume success
   bool bRC = true;

   // Sanity check error reporting name
   if (pName == 0 || pName[0] == 0)
   {
      pName = "?";
   }

   ULONG lineNum = 0;

   // Attempt to open the file
   cDB2TextFile inFile( pStart, nSize );
   if (inFile.IsValid() == true)
   {
      std::string line;
      while (inFile.ReadLine( line ) == true)
      {
         std::string lineCopy = line;
         
         int nLineSize = lineCopy.size();
         LPSTR pLine = new CHAR[ nLineSize + 1 ];
         if (pLine == NULL)
         {
            return false;
         }

         memcpy( pLine, lineCopy.c_str(), nLineSize );

         // Enforce null terminator
         pLine[ nLineSize ] = 0;

         typename Container::mapped_type theType;
         bool bOK = theType.FromString( pLine );
         if (bOK == true)
         {
            // Grab key
            typename Container::key_type theKey = theType.GetKey();

            // Key already exists?
            typename Container::iterator pIter;
            pIter = cont.find( theKey );

            if (pIter != cont.end() && bDuplicatesOK == false)
            {
               // The key already exists which indicates a (recoverable) error
               std::ostringstream tmp;
               tmp << "DB [" << pName << "] Duplicate key, line "
                   << lineNum << " (" << line.c_str() << ")";
               
               log.Log( tmp.str(), eDB2_STATUS_WARNING );

               // Free the current object
               pIter->second.FreeAllocatedStrings();

               // ... and then replace it
               pIter->second = theType;
            }
            else
            {
               typename Container::value_type entry( theKey, theType );
               cont.insert( entry );
            }
         }
         else if (lineCopy.size() > 0)
         {
            // Error parsing line
            std::ostringstream tmp;
            tmp << "DB [" << pName << "] Parsing error, line "
                << lineNum << " (" << line.c_str() << ")";

            log.Log( tmp.str(), eDB2_STATUS_ERROR );

            theType.FreeAllocatedStrings();
            bRC = false;
         }

         delete [] pLine;
         lineNum++;
      }
   }
   else
   {
#ifdef DEBUG
      // Could not open the file
      std::ostringstream tmp;
      tmp << "DB [" << pName << "] Error opening file";

      log.Log( tmp.str(), eDB2_STATUS_WARNING );
#endif

      bRC = false;
   }
   
   return bRC;
};

/*===========================================================================
METHOD:
   FreeDB2Table (Free Public Method)

DESCRIPTION:
   Free up the string allocations in a database table, emptying the
   table in the process
  
PARAMETERS:   
   cont        [ I ] - The database table

RETURN VALUE:
   None
===========================================================================*/
template <class Container>
void FreeDB2Table( Container & cont )
{
   typename Container::iterator pIter = cont.begin();
   while (pIter != cont.end())
   {
      typename Container::mapped_type & theType = pIter->second;
      theType.FreeAllocatedStrings();

      pIter++;
   }

   cont.clear();
};

/*=========================================================================*/
// eDB2EntityType Enumeration
//
//    Database protocol entity header/payload type enumeration
/*=========================================================================*/
enum eDB2EntityType
{
   eDB2_ET_ENUM_BEGIN = -1,   

   eDB2_ET_DIAG_REQ,             // 0  Synchronous request
   eDB2_ET_DIAG_RSP,             // 1  Synchronous response
   eDB2_ET_DIAG_SUBSYS_REQ,      // 2  Synchronous subsystem dispatch request
   eDB2_ET_DIAG_SUBSYS_RSP,      // 3  Synchronous subsystem dispatch response
   eDB2_ET_DIAG_EVENT,           // 4  Asynchronous event
   eDB2_ET_DIAG_LOG,             // 5  Asynchronous log
   eDB2_ET_DIAG_NV_ITEM,         // 6  NVRAM item read/write 
   eDB2_ET_RESERVED7,            // 7  Reserved
   eDB2_ET_RESERVED8,            // 8  Reserved
   eDB2_ET_DIAG_SUBSYS2_REQ,     // 9  Sync subsystem V2 dispatch request
   eDB2_ET_DIAG_SUBSYS2_RSP,     // 10 Sync subsystem V2 dispatch response
   eDB2_ET_DIAG_SUBSYS2_ASYNC,   // 11 Async subsystem V2 dispatch response
   
   eDB2_ET_QMI_BEGIN = 29,       // 29 Start of QMI section

   eDB2_ET_QMI_CTL_REQ,          // 30 QMI CTL request
   eDB2_ET_QMI_CTL_RSP,          // 31 QMI CTL response
   eDB2_ET_QMI_CTL_IND,          // 32 QMI CTL indication
   eDB2_ET_QMI_WDS_REQ,          // 33 QMI WDS request
   eDB2_ET_QMI_WDS_RSP,          // 34 QMI WDS response
   eDB2_ET_QMI_WDS_IND,          // 35 QMI WDS indication
   eDB2_ET_QMI_DMS_REQ,          // 36 QMI DMS request
   eDB2_ET_QMI_DMS_RSP,          // 37 QMI DMS response
   eDB2_ET_QMI_DMS_IND,          // 38 QMI DMS indication
   eDB2_ET_QMI_NAS_REQ,          // 39 QMI NAS request
   eDB2_ET_QMI_NAS_RSP,          // 40 QMI NAS response
   eDB2_ET_QMI_NAS_IND,          // 41 QMI NAS indication
   eDB2_ET_QMI_QOS_REQ,          // 42 QMI QOS request
   eDB2_ET_QMI_QOS_RSP,          // 43 QMI QOS response
   eDB2_ET_QMI_QOS_IND,          // 44 QMI QOS indication 
   eDB2_ET_QMI_WMS_REQ,          // 45 QMI WMS request
   eDB2_ET_QMI_WMS_RSP,          // 46 QMI WMS response
   eDB2_ET_QMI_WMS_IND,          // 47 QMI WMS indication
   eDB2_ET_QMI_PDS_REQ,          // 48 QMI PDS request
   eDB2_ET_QMI_PDS_RSP,          // 49 QMI PDS response
   eDB2_ET_QMI_PDS_IND,          // 50 QMI PDS indication
   eDB2_ET_QMI_AUTH_REQ,         // 51 QMI AUTH request
   eDB2_ET_QMI_AUTH_RSP,         // 52 QMI AUTH response
   eDB2_ET_QMI_AUTH_IND,         // 53 QMI AUTH indication
   eDB2_ET_QMI_CAT_REQ,          // 54 QMI CAT request
   eDB2_ET_QMI_CAT_RSP,          // 55 QMI CAT response
   eDB2_ET_QMI_CAT_IND,          // 56 QMI CAT indication
   eDB2_ET_QMI_RMS_REQ,          // 57 QMI RMS request
   eDB2_ET_QMI_RMS_RSP,          // 58 QMI RMS response
   eDB2_ET_QMI_RMS_IND,          // 59 QMI RMS indication
   eDB2_ET_QMI_OMA_REQ,          // 60 QMI OMA request
   eDB2_ET_QMI_OMA_RSP,          // 61 QMI OMA response
   eDB2_ET_QMI_OMA_IND,          // 62 QMI OMA indication
   eDB2_ET_QMI_VOICE_REQ,        // 63 QMI voice request
   eDB2_ET_QMI_VOICE_RSP,        // 64 QMI voice response
   eDB2_ET_QMI_VOICE_IND,        // 65 QMI voice indication

   eDB2_ET_QMI_END,              // 63 End of QMI section

   eDB2_ET_ENUM_END  

};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2EntityType validity check

PARAMETERS:
   et          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2EntityType et )
{
   bool retVal = false;
   if ( (et > eDB2_ET_ENUM_BEGIN && et <= eDB2_ET_DIAG_SUBSYS2_ASYNC)
   ||   (et > eDB2_ET_QMI_BEGIN && et < eDB2_ET_QMI_END) )

   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsDiagEntityType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the DIAG protocol?

PARAMETERS:
   entityType  [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsDiagEntityType( eDB2EntityType entityType )
{
   bool retVal = false;
   if (entityType == eDB2_ET_DIAG_REQ 
   ||  entityType == eDB2_ET_DIAG_RSP
   ||  entityType == eDB2_ET_DIAG_SUBSYS_REQ
   ||  entityType == eDB2_ET_DIAG_SUBSYS_RSP
   ||  entityType == eDB2_ET_DIAG_EVENT
   ||  entityType == eDB2_ET_DIAG_LOG
   ||  entityType == eDB2_ET_DIAG_NV_ITEM
   ||  entityType == eDB2_ET_DIAG_SUBSYS2_REQ
   ||  entityType == eDB2_ET_DIAG_SUBSYS2_RSP
   ||  entityType == eDB2_ET_DIAG_SUBSYS2_ASYNC)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsDiagEntityRequestType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the DIAG protocol and if so
   does it represent a request?

PARAMETERS:
   entityType  [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsDiagEntityRequestType( eDB2EntityType entityType )
{
   bool retVal = false;
   if (entityType == eDB2_ET_DIAG_REQ 
   ||  entityType == eDB2_ET_DIAG_SUBSYS_REQ
   ||  entityType == eDB2_ET_DIAG_SUBSYS2_REQ)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsDiagEntityResponseType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the DIAG protocol and if so
   does it represent a response?

PARAMETERS:
   entityType  [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsDiagEntityResponseType( eDB2EntityType entityType )
{
   bool retVal = false;
   if (entityType == eDB2_ET_DIAG_RSP 
   ||  entityType == eDB2_ET_DIAG_SUBSYS_RSP
   ||  entityType == eDB2_ET_DIAG_SUBSYS2_RSP)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsDiagEntityAsyncType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the DIAG protocol and if so
   does it represent asynchronous incoming data?

PARAMETERS:
   entityType  [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsDiagEntityAsyncType( eDB2EntityType entityType )
{
   bool retVal = false;
   if (entityType == eDB2_ET_DIAG_EVENT 
   ||  entityType == eDB2_ET_DIAG_LOG
   ||  entityType == eDB2_ET_DIAG_SUBSYS2_ASYNC)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsQMIEntityType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the QMI protocol?

PARAMETERS:
   et          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsQMIEntityType( eDB2EntityType et )
{
   bool retVal = false;
   if (et > eDB2_ET_QMI_BEGIN && et < eDB2_ET_QMI_END)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsQMIEntityRequestType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the QMI protocol and if so
   does it represent a request?

PARAMETERS:
   entityType  [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsQMIEntityRequestType( eDB2EntityType entityType )
{
   bool retVal = false;

   // One QMI service is always a triplet of REQ/RSP/IND
   DWORD baseVal = (DWORD)eDB2_ET_QMI_BEGIN + 1;
   if ((DWORD)entityType % baseVal == 0)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsQMIEntityResponseType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the QMI protocol and if so
   does it represent a response?

PARAMETERS:
   entityType  [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsQMIEntityResponseType( eDB2EntityType entityType )
{
   bool retVal = false;

   // One QMI service is always a triplet of REQ/RSP/IND
   DWORD baseVal = (DWORD)eDB2_ET_QMI_BEGIN + 1;
   if ((DWORD)entityType % baseVal == 1)
   {
      retVal = true;
   }

   return retVal;
};

/*===========================================================================
METHOD:
   IsQMIEntityIndicationType (Inline Method)

DESCRIPTION:
   Does the eDB2EntityType value represent the QMI protocol and if so
   does it represent an indication?

PARAMETERS:
   entityType  [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsQMIEntityIndicationType( eDB2EntityType entityType )
{
   bool retVal = false;

   // One QMI service is always a triplet of REQ/RSP/IND
   DWORD baseVal = (DWORD)eDB2_ET_QMI_BEGIN + 1;
   if ((DWORD)entityType % baseVal == 2)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eDB2FragmentType Enumeration
//
//    Database fragment type enumeration - determines in what table
//    (or manner) the fragment is described
/*=========================================================================*/
enum eDB2FragmentType
{
   eDB2_FRAGMENT_TYPE_ENUM_BEGIN = -1,  

   eDB2_FRAGMENT_FIELD,                // 0  Simple field fragment
   eDB2_FRAGMENT_STRUCT,               // 1  Structure fragment
   eDB2_FRAGMENT_CONSTANT_PAD,         // 2  Pad fragment, fixed length (bits)
   eDB2_FRAGMENT_VARIABLE_PAD_BITS,    // 3  Pad fragment, variable (bits)
   eDB2_FRAGMENT_VARIABLE_PAD_BYTES,   // 4  Pad fragment, variable (bytes)
   eDB2_FRAGMENT_FULL_BYTE_PAD,        // 5  Pad fragment, pad to a full byte
   eDB2_FRAGMENT_MSB_2_LSB,            // 6  Switch to MSB -> LSB order
   eDB2_FRAGMENT_LSB_2_MSB,            // 7  Switch to LSB -> MSB order

   eDB2_FRAGMENT_TYPE_ENUM_END 
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2FragmentType validity check

PARAMETERS:
   fragType    [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2FragmentType fragType )
{
   bool retVal = false;
   if (fragType > eDB2_FRAGMENT_TYPE_ENUM_BEGIN 
   &&  fragType < eDB2_FRAGMENT_TYPE_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eDB2ModifierType Enumeration
//
//    Database fragment modifier type enumeration - determines how a
//    fragment is modified/used
/*=========================================================================*/
enum eDB2ModifierType
{
   eDB2_MOD_TYPE_ENUM_BEGIN = -1, 

   eDB2_MOD_NONE,             // 0  Modifier is not used
   eDB2_MOD_CONSTANT_ARRAY,   // 1  Constant (elements) array
   eDB2_MOD_VARIABLE_ARRAY,   // 2  Variable (elements) array
   eDB2_MOD_OBSOLETE_3,       // 3  Constant (bits) array [OBS]
   eDB2_MOD_OBSOLETE_4,       // 4  Variable (bits) array [OBS]
   eDB2_MOD_OPTIONAL,         // 5  Fragment is optional
   eDB2_MOD_VARIABLE_ARRAY2,  // 6  Variable (elements) array, start/stop given
   eDB2_MOD_VARIABLE_ARRAY3,  // 7  Variable (elements) array, simple expression
   eDB2_MOD_VARIABLE_STRING1, // 8  Variable length string (bit length)
   eDB2_MOD_VARIABLE_STRING2, // 9  Variable length string (byte length)
   eDB2_MOD_VARIABLE_STRING3, // 10 Variable length string (character length)

   eDB2_MOD_TYPE_ENUM_END   
};

/*===========================================================================
METHOD:
   ModifiedToArray (Inline Method)

DESCRIPTION:
   Does this modifier indicate an array?

PARAMETERS:
   modType         [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool ModifiedToArray( eDB2ModifierType modType )
{
   bool bRC = false;
   if ( (modType == eDB2_MOD_CONSTANT_ARRAY)
   ||   (modType == eDB2_MOD_VARIABLE_ARRAY)
   ||   (modType == eDB2_MOD_VARIABLE_ARRAY2)
   ||   (modType == eDB2_MOD_VARIABLE_ARRAY3) )
   {
      bRC = true;
   }

   return bRC;
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2ModifierType validity check

PARAMETERS:
   modType     [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2ModifierType modType )
{
   bool retVal = false;
   if (modType > eDB2_MOD_TYPE_ENUM_BEGIN 
   &&  modType < eDB2_MOD_TYPE_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eDB2FieldType Enumeration
//
//    Database field type enumeration - determines whether the field in
//    question is a standard type or an enumeration
/*=========================================================================*/
enum eDB2FieldType
{
   eDB2_FIELD_TYPE_ENUM_BEGIN = -1,  

   eDB2_FIELD_STD,            // 0  Field is a standard type (see below)
   eDB2_FIELD_ENUM_UNSIGNED,  // 1  Field is an unsigned enumerated type
   eDB2_FIELD_ENUM_SIGNED,    // 2  Field is a signed enumerated type

   eDB2_FIELD_TYPE_ENUM_END 
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2FieldType validity check

PARAMETERS:
   fieldType   [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2FieldType fieldType )
{
   bool retVal = false;
   if (fieldType > eDB2_FIELD_TYPE_ENUM_BEGIN 
   &&  fieldType < eDB2_FIELD_TYPE_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eDB2StdFieldType Enumeration
//
//    Database standard field type enumeration
/*=========================================================================*/
enum eDB2StdFieldType
{
   eDB2_FIELD_STDTYPE_ENUM_BEGIN = -1,  

   eDB2_FIELD_STDTYPE_BOOL,        // 0  Field is a boolean (0/1, false/true)
   eDB2_FIELD_STDTYPE_INT8,        // 1  Field is 8-bit signed integer
   eDB2_FIELD_STDTYPE_UINT8,       // 2  Field is 8-bit unsigned integer
   eDB2_FIELD_STDTYPE_INT16,       // 3  Field is 16-bit signed integer
   eDB2_FIELD_STDTYPE_UINT16,      // 4  Field is 16-bit unsigned integer
   eDB2_FIELD_STDTYPE_INT32,       // 5  Field is 32-bit signed integer
   eDB2_FIELD_STDTYPE_UINT32,      // 6  Field is 32-bit unsigned integer
   eDB2_FIELD_STDTYPE_INT64,       // 7  Field is 64-bit signed integer
   eDB2_FIELD_STDTYPE_UINT64,      // 8  Field is 64-bit unsigned integer
   eDB2_FIELD_STDTYPE_STRING_A,    // 9  ANSI fixed length string
   eDB2_FIELD_STDTYPE_STRING_U,    // 10 UNICODE fixed length string
   eDB2_FIELD_STDTYPE_STRING_ANT,  // 11 ANSI NULL terminated string
   eDB2_FIELD_STDTYPE_STRING_UNT,  // 12 UNICODE NULL terminated string
   eDB2_FIELD_STDTYPE_FLOAT32,     // 13 Field is 32-bit floating point value
   eDB2_FIELD_STDTYPE_FLOAT64,     // 14 Field is 64-bit floating point value
   eDB2_FIELD_STDTYPE_STRING_U8,   // 15 UTF-8 encoded fixed length string
   eDB2_FIELD_STDTYPE_STRING_U8NT, // 16 UTF-8 encoded NULL terminated string

   eDB2_FIELD_STDTYPE_ENUM_END 
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2StdFieldType validity check

PARAMETERS:
   fieldType   [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2StdFieldType fieldType )
{
   bool retVal = false;
   if (fieldType > eDB2_FIELD_STDTYPE_ENUM_BEGIN 
   &&  fieldType < eDB2_FIELD_STDTYPE_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eDB2Operator Enumeration
//
//    Database conditional fragment operator type enumeration
/*=========================================================================*/
enum eDB2Operator
{
   eDB2_OP_TYPE_ENUM_BEGIN = -1,

   eDB2_OP_LT,
   eDB2_OP_LTE,
   eDB2_OP_EQ,
   eDB2_OP_NEQ,
   eDB2_OP_GTE,
   eDB2_OP_GT,
   eDB2_OP_DIV,
   eDB2_OP_NDIV,

   eDB2_OP_TYPE_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2Operator validity check

PARAMETERS:
   op          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2Operator op )
{
   bool retVal = false;
   if (op > eDB2_OP_TYPE_ENUM_BEGIN 
   &&  op < eDB2_OP_TYPE_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// eDB2ExpOperator Enumeration
//
//    Database simple expression operator type enumeration
/*=========================================================================*/
enum eDB2ExpOperator
{
   eDB2_EXPOP_TYPE_ENUM_BEGIN = -1,

   eDB2_EXPOP_ADD,
   eDB2_EXPOP_SUB,
   eDB2_EXPOP_MUL,
   eDB2_EXPOP_DIV,
   eDB2_EXPOP_REM,
   eDB2_EXPOP_MIN,
   eDB2_EXPOP_MAX,

   eDB2_EXPOP_TYPE_ENUM_END
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   eDB2ExpOperator validity check

PARAMETERS:
   op          [ I ] - Enum value being verified

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( eDB2ExpOperator op )
{
   bool retVal = false;
   if (op > eDB2_EXPOP_TYPE_ENUM_BEGIN 
   &&  op < eDB2_EXPOP_TYPE_ENUM_END)
   {
      retVal = true;
   }

   return retVal;
};

/*=========================================================================*/
// Struct sDB2ProtocolEntity
//
//    Structure that defines the schema for the protocol entity table
/*=========================================================================*/
struct sDB2ProtocolEntity
{
   public:
      // (Inline) Default constructor
      sDB2ProtocolEntity()
         :  mType( eDB2_ET_ENUM_BEGIN ),
            mStructID( -1 ),
            mFormatID( -1 ),
            mbInternal( false ),
            mFormatExID( -1 ),
            mpName( EMPTY_STRING )
      { };

      // (Inline) Free up our allocated strings
      void FreeAllocatedStrings()
      {
         if (mpName != 0 && mpName != EMPTY_STRING)
         {
            delete [] mpName;
            mpName = 0;
         }
      };

      // (Inline) Return object key
      std::vector <ULONG> GetKey() const
      {
         return mID;
      };

      // Populate this object from a string
      bool FromString( LPSTR pStr );

      // Is this object valid?
      bool IsValid() const;

      /* Type of protocol entity 'header/payload' */
      eDB2EntityType mType;

      /* Multi-value ID (includes above type) */
      std::vector <ULONG> mID;

      /* Associated structure ID (-1 = no structure) */
      int mStructID;

      /* Associated format specifier (-1 = none) */
      int mFormatID;

      /* Is this protocol entity internal only? */
      bool mbInternal;

      /* Associated extended format specifier (-1 = none) */
      int mFormatExID;

      /* Name of protocol entity */
      LPCSTR mpName;
};

/*=========================================================================*/
// Struct sDB2Fragment
//
//    Structure that defines the schema for the protocol structure table
/*=========================================================================*/
struct sDB2Fragment
{
   public:
      // (Inline) Default constructor
      sDB2Fragment()
         :  mStructID( 0 ),
            mFragmentOrder( 0 ),
            mFragmentOffset( 0 ),
            mFragmentType( eDB2_FRAGMENT_TYPE_ENUM_BEGIN ),
            mFragmentValue( 0 ),
            mModifierType( eDB2_MOD_TYPE_ENUM_BEGIN ),
            mpModifierValue( EMPTY_STRING ),
            mpName( EMPTY_STRING )
      { };

      // (Inline) Free up our allocated strings
      void FreeAllocatedStrings()
      {
         if (mpName != 0 && mpName != EMPTY_STRING)
         {
            delete [] mpName;
            mpName = 0;
         }

         if (mpModifierValue != 0 && mpModifierValue != EMPTY_STRING)
         {
            delete [] mpModifierValue;
            mpModifierValue = 0;
         }
      };

      // (Inline) Return object key
      std::pair <ULONG, ULONG> GetKey() const
      {
         std::pair <ULONG, ULONG> key( mStructID, mFragmentOrder );
         return key;
      };

      // Populate this object from a string
      bool FromString( LPSTR pStr );

      // Is this object valid?
      bool IsValid() const;

      // Build a simple condition string
      static std::string BuildCondition( 
         ULONG                      id,
         eDB2Operator               op,
         LONGLONG                   val,
         bool                       bF2F );

      // Evaluate a simple condition
      static bool EvaluateCondition( 
         LONGLONG                   valA,
         eDB2Operator               op,
         LONGLONG                   valB );      

      // Parse a simple condition
      static bool ParseCondition( 
         LPCSTR                    pCondition,
         ULONG &                    id,
         eDB2Operator &             op,
         LONGLONG &                 val,
         bool &                     bF2F );

      // Build a simple expression string
      static std::string BuildExpression( 
         ULONG                      id,
         eDB2ExpOperator            op,
         LONGLONG                   val,
         bool                       bF2F );

      // Evaluate a simple expression
      static bool EvaluateExpression( 
         LONGLONG                   valA,
         eDB2ExpOperator            op,
         LONGLONG                   valB,
         LONGLONG &                 res );      

      // Parse a simple expression
      static bool ParseExpression( 
         LPCSTR                    pExpr,
         ULONG &                    id,
         eDB2ExpOperator &          op,
         LONGLONG &                 val,
         bool &                     bF2F );

      /* Enclosing structure ID */
      ULONG mStructID;

      /* Order of fragment within structure */
      ULONG mFragmentOrder;

      /* Offset (in bits) of fragment from beginning of enclosing structure */
      int mFragmentOffset;

      /* Fragment type, how to interpret the following fragment value */
      eDB2FragmentType mFragmentType;

      /* Fragment Value */
      ULONG mFragmentValue;

      /* Modifier type, how to interpret the following modifier value */
      eDB2ModifierType mModifierType;

      /* Modifier value */
      LPCSTR mpModifierValue;

      /* Fragment Name */
      LPCSTR mpName;
};

/*=========================================================================*/
// Struct sDB2Field
//
//    Structure that defines the schema for the protocol field table
/*=========================================================================*/
struct sDB2Field
{
   public:
      // (Inline) Default constructor
      sDB2Field()
         :  mID( 0 ),
            mSize( 0 ),
            mType( eDB2_FIELD_TYPE_ENUM_BEGIN ),
            mTypeVal( 0 ),
            mbHex( false ),
            mbInternal( false ),
            mDescriptionID( -1 ),
            mpName( EMPTY_STRING )
      { };

      // (Inline) Free up our allocated strings
      void FreeAllocatedStrings()
      {
         if ( (mpName != 0) 
         &&   (mpName != EMPTY_STRING) )
         {
            delete [] mpName;
            mpName = 0;
         }
      };

      // (Inline) Return object key
      ULONG GetKey() const
      {
         return mID;
      };

      // Populate this object from a string
      bool FromString( LPSTR pStr );
 
      // Is this object valid?
      bool IsValid() const;

      /* Field ID */
      ULONG mID;

      /* Size of field (in bits, maximum is 64 bits for integral types) */
      ULONG mSize;

      /* Field type */
      eDB2FieldType mType;

      /* Actual field type (eDB2StdFieldType or enum ID) */
      ULONG mTypeVal;

      /* Display integral fields as hexadecimal? */
      bool mbHex;

      /* Is this field internal only? */
      bool mbInternal;

      /* Description of field */
      int mDescriptionID;

      /* Field name */
      LPCSTR mpName;
};

/*=========================================================================*/
// Struct sDB2Category
//
//    Structure that defines the generic category table schema, gives 
//    category ID, category name, category description, and parent 
//    category relationship (specified as a category ID into the very 
//    same table)
/*=========================================================================*/
struct sDB2Category
{
   public:
      // (Inline) Default constructor
      sDB2Category()
         :  mID( 0 ),
            mParentID( -1 ),            
            mpName( EMPTY_STRING ),
            mDescriptionID( -1 )
      { };

      // (Inline) Free up our allocated strings
      void FreeAllocatedStrings()
      {
         if (mpName != 0 && mpName != EMPTY_STRING)
         {
            delete [] mpName;
            mpName = 0;
         }
      };

      // (Inline) Return object key
      ULONG GetKey() const
      {
         return mID;
      };

      // Populate this object from a string
      bool FromString( LPSTR pStr );
 
      // Is this object valid?
      bool IsValid() const;

      /* Category ID */
      ULONG mID;

      /* Category ID of parent, -1 implies no parent/category is at root */
      int mParentID;

      /* Category display name */
      LPCSTR mpName;

      /* Description of category */
      int mDescriptionID;
};


/*===========================================================================
METHOD:
   ValidateDB2Categories (Public Method)

DESCRIPTION:
   Validate the relationship between a pair of DB category/reference tables
  
   NOTE: Discovered problems will be repaired, i.e. bogus/problematic
         category IDs are reset to -1

PARAMETERS:   
   catMap      [ I ] - The category table
   refMap      [ I ] - The reference table
   pName       [ I ] - Table name (for error reporting)
   log         [ I ] - Error log

RETURN VALUE:
   bool
===========================================================================*/
template <class Key, class NamedType>
bool ValidateDB2Categories( 
   std::map <ULONG, sDB2Category> & catMap,
   std::map <Key, NamedType> &      refMap,
   LPCSTR                           pName,
   cDB2StatusLog &                  log = gDB2DefaultLog )
{
   // Assume success
   bool bRC = true;
   std::string err;

   // Sanity check table name
   if (pName == 0 || pName[0] == 0)
   {
      pName = "?";
   }

   // First validate/repair category map; stage 1: bad parent IDs
   std::map <ULONG, sDB2Category>::iterator pCats = catMap.begin();
   while (pCats != catMap.end())
   {
      sDB2Category & cat = pCats->second;
      pCats++;

      if (cat.IsValid() == false)
      {
         continue;
      }

      // The parent ID must be -1 or exist in the category map
      if (cat.mParentID != -1)
      {
         if (catMap.find( cat.mParentID ) == catMap.end())
         {
            // Unable to locate parent category
            std::ostringstream tmp;
            tmp << "DB [" << pName << "] Missing ID, parent ID "
                << cat.mParentID;

            log.Log( tmp.str(), eDB2_STATUS_ERROR );

            cat.mParentID = -1;
            bRC = false;
         }
      }      
   }

   // Validate/repair category map; stage 2: loop detection
   pCats = catMap.begin();
   while (pCats != catMap.end())
   {
      std::set <int> catsVisited;
      sDB2Category & cat = pCats->second;

      // Itererate up through parents
      int parentID = cat.mParentID;
      while (parentID != -1)
      {
         // Have we already been here?
         if (catsVisited.find( parentID ) == catsVisited.end())
         {
            // Nope, add ID and go on to the next one
            catsVisited.insert( parentID );

            std::map <ULONG, sDB2Category>::iterator pParent;
            pParent = catMap.find( parentID );

            parentID = pParent->second.mParentID;
         }
         else
         {
            // Yes, we are caught in a loop
            std::ostringstream tmp;
            tmp << "DB [" << pName << "]  Loop in category, parent ID "
                << cat.mParentID;

            log.Log( tmp.str(), eDB2_STATUS_ERROR );

            cat.mParentID = -1;
            bRC = false;

            break;
         }
      }

      pCats++;
   }

   // Validate that each reference references valid category IDs
   typename std::map <Key, NamedType>::iterator pTypes = refMap.begin();
   while (pTypes != refMap.end())
   {
      NamedType & theType = pTypes->second;
      std::set <int> cats = theType.mCategoryIDs;

      std::set <int>::iterator pRefCats = cats.begin();
      while (pRefCats != cats.end())
      {
         if (*pRefCats != -1)
         {
            pCats = catMap.find( *pRefCats );
            if (pCats == catMap.end())
            {
               // Unable to locate category 
               std::ostringstream tmp;
               tmp << "DB [" << pName << "]  Missing ID, category ID "
                   << *pRefCats << ", reference " << theType.mpName;

               log.Log( tmp.str(), eDB2_STATUS_ERROR );

               *pRefCats = -1;
               bRC = false;
            }
         }

         pRefCats++;
      }

      pTypes++;
   }

   return bRC;
};

/*=========================================================================*/
// Struct sDB2NVItem
//
//    NVRAM item structure for database schema
/*=========================================================================*/
struct sDB2NVItem
{
   public:
      // (Inline) Default constructor
      sDB2NVItem()
         :  mItem( 0 ),
            mpName( EMPTY_STRING ),
            mDescriptionID( -1 )
      { };

      // (Inline) Free up our allocated strings
      void FreeAllocatedStrings()
      {
         if (mpName != 0 && mpName != EMPTY_STRING)
         {
            delete [] mpName;
            mpName = 0;
         }
      };

      // (Inline) Return object key
      ULONG GetKey() const
      {
         return mItem;
      };

      // Populate this object from a string
      bool FromString( LPSTR pStr );
 
      // Is this object valid?
      bool IsValid() const;

      /* Category IDs (indices into NV items category table) */
      std::set <int> mCategoryIDs;
    
      /* Item number */
      ULONG mItem;

      /* NV item display name */
      LPCSTR mpName;

      /* Description of NV item */
      int mDescriptionID;
};

/*=========================================================================*/
// Struct sDB2Enum
//
//    Structure that defines the schema for the enum table
/*=========================================================================*/
struct sDB2Enum
{
   public:
      // (Inline) Default constructor
      sDB2Enum()
         :  mID( 0 ),
            mbInternal( false ),
            mpName( EMPTY_STRING ),
            mDescriptionID( -1 )
      { };

      // (Inline) Free up our allocated strings
      void FreeAllocatedStrings()
      {
         if (mpName != 0 && mpName != EMPTY_STRING)
         {
            delete [] mpName;
            mpName = 0;
         }
      };

      // (Inline) Return object key
      ULONG GetKey() const
      {
         return mID;
      };

      // Populate this object from a string
      bool FromString( LPSTR pStr );

      // Is this object valid?
      bool IsValid() const;

      /* Enum ID */
      ULONG mID;

      /* Is this enum used internally? */
      bool mbInternal;

      /* Description of enum */
      int mDescriptionID;

      /* Name of enum */
      LPCSTR mpName;
};

/*=========================================================================*/
// Struct sDB2EnumEntry
//
//    Structure that defines the schema for the enum entry table
/*=========================================================================*/
struct sDB2EnumEntry
{
   public:
      // (Inline) Default constructor
      sDB2EnumEntry()
         :  mID( 0 ),
            mValue( -1 ),
            mbHex( false ),
            mpName( EMPTY_STRING ),
            mDescriptionID( -1 )
      { };

      // (Inline) Free up our allocated strings
      void FreeAllocatedStrings()
      {
         if (mpName != 0 && mpName != EMPTY_STRING)
         {
            delete [] mpName;
            mpName = 0;
         }
      };

      // (Inline) Return object key
      std::pair <ULONG, int> GetKey() const
      {
         std::pair <ULONG, int> key( mID, mValue );
         return key;
      };

      // (Inline) Populate this object from a string
      bool FromString( LPSTR pStr );

      // Is this object valid?
      bool IsValid() const;

      /* Enum ID */
      ULONG mID;

      /* Enum entry value */
      int mValue;

      /* Hexadecimal flag */
      bool mbHex;

      /* Enum value name */
      LPCSTR mpName;

      /* Description of enum value */
      int mDescriptionID;
};

/*=========================================================================*/
// Struct sDB2SimpleCondition
//
//    Structure that defines a (parsed) simple condition modifier
/*=========================================================================*/
struct sDB2SimpleCondition
{
   public:
      // (Inline) Default constructor
      sDB2SimpleCondition()
         :  mID( 0 ),
            mOperator( eDB2_OP_TYPE_ENUM_BEGIN ),
            mValue( 0 ),
            mbF2F( false )
      { };

      // (Inline) Is this object valid?
      bool IsValid() const
      {
         return ::IsValid( mOperator );
      };

      /* ID of field whose value is to be used */
      ULONG mID;

      /* Operator to be used */
      eDB2Operator mOperator;

      /* Value (or field ID) to compare against */
      LONGLONG mValue;

      /* Field to field expression? */
      bool mbF2F;
};

/*=========================================================================*/
// Struct sDB2SimpleExpression
//
//    Structure that defines a (parsed) simple expression
/*=========================================================================*/
struct sDB2SimpleExpression
{
   public:
      // (Inline) Default constructor
      sDB2SimpleExpression()
         :  mID( 0 ),
            mOperator( eDB2_EXPOP_TYPE_ENUM_BEGIN ),
            mValue( 0 ),
            mbF2F( false )
      { };

      // (Inline) Is this object valid?
      bool IsValid() const
      {
         return (::IsValid( mOperator ) && mValue != 0);
      };

      /* ID of field whose value is to be used */
      ULONG mID;

      /* Operator to be used */
      eDB2ExpOperator mOperator;

      /* Value (or field ID) to compare against */
      LONGLONG mValue;

      /* Field to field expression? */
      bool mbF2F;
};

/*=========================================================================*/
// Struct sLPCSTRCmp
//
//    Structure that defines the '<' operator for string comparison
/*=========================================================================*/
struct sLPCSTRCmp
{
   public:
      // (Inline) Is A < B?
      bool operator () (
         LPCSTR                    pStrA,
         LPCSTR                    pStrB ) const
      {
         bool bLess = false;
         if (pStrA != 0 && pStrB != 0)
         {
            bLess = (strcmp( pStrA, pStrB ) < 0);
         }

         return bLess;
      };
};

/*=========================================================================*/
//  Case insensitive compare function
/*=========================================================================*/
inline bool InsensitiveCompare( CHAR first, CHAR second )
{
   return tolower( first ) < tolower( second );
}

/*=========================================================================*/
// Struct sLPCSTRCmpI
//
//    Structure that defines the '<' operator for string comparison
//    (case insensitive version)
/*=========================================================================*/
struct sLPCSTRCmpI
{
   public:
      // (Inline) Is A < B?
      bool operator () (
         LPCSTR                    pStrA,
         LPCSTR                    pStrB ) const
      {
         bool bLess = false;
         if (pStrA != 0 && pStrB != 0)
         {
            // Is there a simpler stl function for this?
            bLess = std::lexicographical_compare( pStrA,
                                                  pStrA + 
                                                  strlen( pStrA ),
                                                  pStrB,
                                                  pStrB + 
                                                  strlen( pStrB ),
                                                  InsensitiveCompare );
         }

         return bLess;
      };
};

//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------

// The protocol entity table expressed as a type
typedef std::multimap <std::vector <ULONG>, sDB2ProtocolEntity> tDB2EntityMap;

// Protocol entity entity name to ID (reverse) table
typedef std::map <LPCSTR, std::vector <ULONG>, sLPCSTRCmpI> tDB2EntityNameMap;

// The struct table expressed as a type
typedef std::map <std::pair <ULONG, ULONG>, sDB2Fragment> tDB2FragmentMap;

// The field table expressed as a type
typedef std::map <ULONG, sDB2Field> tDB2FieldMap;

// A generic category table expressed as a type
typedef std::map <ULONG, sDB2Category> tDB2CategoryMap;

// NV item table expressed as a map type
typedef std::map <ULONG, sDB2NVItem> tDB2NVMap;

// Enum table expressed as a map type
typedef std::map <ULONG, sDB2Enum> tDB2EnumNameMap;

// Enum entry table expressed as a map type
typedef std::map <std::pair <ULONG, int>, sDB2EnumEntry> tDB2EnumEntryMap;

// The built enumeration map
typedef std::pair < ULONG, std::map <int, LPCSTR> > tDB2EnumMapPair;
typedef std::map <LPCSTR, tDB2EnumMapPair, sLPCSTRCmp> tDB2EnumMap;

// Parsed fragment modifier map - optional fragment
typedef std::map <LPCSTR, sDB2SimpleCondition> tDB2OptionalModMap;

// Parsed fragment modifier map - simple expression based sizes
typedef std::map <LPCSTR, sDB2SimpleExpression> tDB2ExpressionModMap;

// Parsed fragment modifier map - element count specified arrays
typedef std::map <LPCSTR, ULONG> tDB2Array1ModMap;

// Parsed fragment modifier map - start/stop index specified arrays
typedef std::map <LPCSTR, std::pair <ULONG, ULONG> > tDB2Array2ModMap;

// A protocol entity navigation map expressed as a type
typedef std::map <std::vector <ULONG>, cDB2NavTree *> tDB2EntityNavMap;



/*=========================================================================*/
// Class cCoreDatabase
/*=========================================================================*/
class cCoreDatabase
{
   public:
      // Constructor
      cCoreDatabase();

      // Destructor
      virtual ~cCoreDatabase();

      // Initialize the database - must be done once (and only once) prior
      // to any database object access
      virtual bool Initialize( LPCSTR pBasePath );
      virtual bool Initialize();

      // Exit (cleanup) the database
      virtual void Exit();

      // Get the entity navigation tree for the given protocol entity, if
      // none exists one will be built and returned
      const cDB2NavTree * GetEntityNavTree( 
         const std::vector <ULONG> &   key ) const;

      // Find the protocol entity with the specified key
      bool FindEntity( 
         const std::vector <ULONG> &   key,
         sDB2ProtocolEntity &          entity ) const;

      // Find the protocol entity with the specified name
      bool FindEntity( 
         LPCSTR                    pEntityName,
         sDB2ProtocolEntity &       entity ) const;

      // Map a protocol entity name to an ID
      bool MapEntityNameToID( 
         LPCSTR                    pName,
         std::vector <ULONG> &      key ) const;

      // Map the given enum value (specified by enum ID, and enum value) 
      // to the enum value name string
      std::string MapEnumToString( 
         ULONG                      enumID,
         int                        enumVal,
         bool                       bSimpleErrFmt = false,
         bool                       bHex = false ) const;

      // Map the given enum value (specified by enum name, and enum value) 
      // to the enum value name string
      std::string MapEnumToString( 
         LPCSTR                    pEnumName,
         int                        enumVal,
         bool                       bSimpleErrFmt = false,
         bool                       bHex = false ) const;

      // (Inline) Set status log (object must exist for the duration of 
      // the DB or at least until being reset)
      void SetLog( cDB2StatusLog * pLog )
      {
         if (pLog != 0)
         {
            mpLog = pLog;
         }
      };

      // (Inline) Return protocol entities
      const tDB2EntityMap & GetProtocolEntities() const
      {
         return mProtocolEntities;
      };

      // (Inline) Return protocol entity names
      const tDB2EntityNameMap & GetProtocolEntityNames() const
      {
         return mEntityNames;
      };

      // (Inline) Return protocol structures
      const tDB2FragmentMap & GetProtocolStructs() const
      {
         return mEntityStructs;
      };

      // (Inline) Return protocol fields
      const tDB2FieldMap & GetProtocolFields() const
      {
         return mEntityFields;
      };

      // (Inline) Return assembled enumeration map
      const tDB2EnumMap & GetEnums() const
      {
         return mEnumMap;
      };

      // (Inline) Return raw enumeration map
      const tDB2EnumNameMap & GetRawEnums() const 
      {
         return mEnumNameMap;
      };

      // (Inline) Return raw enumeration entry map
      const tDB2EnumEntryMap & GetRawEnumEntries() const 
      {
         return mEnumEntryMap;
      };

      // (Inline) Return parsed fragment modifier map - optional
      const tDB2OptionalModMap & GetOptionalMods() const
      {
         return mOptionalModMap;
      };

      // (Inline) Return parsed fragment modifier map - expressions
      const tDB2ExpressionModMap & GetExpressionMods() const
      {
         return mExpressionModMap;
      };

      // (Inline) Return parsed fragment modifier map - element 
      // count specified arrays
      const tDB2Array1ModMap & GetArray1Mods() const
      {
         return mArray1ModMap;
      };

      // (Inline) Return parsed fragment modifier map - start/stop 
      // index specified arrays
      const tDB2Array2ModMap & GetArray2Mods() const
      {
         return mArray2ModMap;
      };

   protected:
      // Assemble the internal enum map
      bool AssembleEnumMap();

      // Assemble the internal protocol entity name map
      bool AssembleEntityNameMap();

      // Build the modifier tables
      bool BuildModifierTables();

      // Check and set the passed in path to something that is useful
      std::string CheckAndSetBasePath( LPCSTR pBasePath ) const;

      // Load all tables related to structure (entity, struct, field)
      bool LoadStructureTables( LPCSTR pBasePath );
      bool LoadStructureTables();

      // Load all enumeration related tables
      bool LoadEnumTables( LPCSTR pBasePath );
      bool LoadEnumTables();

      // Validate (and attempt repair of) structure related tables
      bool ValidateStructures();

      // Validate a single structure
      bool ValidateStructure(
         ULONG                      structID,
         std::set <ULONG> &         fields,
         ULONG                      depth );

      // Validate a single field
      bool ValidateField(
         ULONG                      structID,
         ULONG                      fieldID,
         std::set <ULONG> &         fields );

      // Validate an array specifier
      bool ValidateArraySpecifier(
         const sDB2Fragment &       frag,
         const std::set <ULONG> &   fields );

      // Validate a simple optional fragment specifier
      bool ValidateOptionalSpecifier(
         const sDB2Fragment &       frag,
         const std::set <ULONG> &   fields );

      // Validate a simple expression fragment specifier
      bool ValidateExpressionSpecifier(
         const sDB2Fragment &       frag,
         const std::set <ULONG> &   fields );

      /* Status log */
      cDB2StatusLog * mpLog;

      /* Protocol entity table, referenced by multi-value key */
      tDB2EntityMap mProtocolEntities;

      /* Protocol entity keys, referenced by indexed by entity name */
      tDB2EntityNameMap mEntityNames;

      /* The on-demand Protocol entity navigation map */
      mutable tDB2EntityNavMap mEntityNavMap;

      /* Protocol entity struct table, indexed by struct ID & fragment order */
      tDB2FragmentMap mEntityStructs;

      /* Protocol entity field table, indexed by field ID */ 
      tDB2FieldMap mEntityFields;

      /* Enum map, indexed by enum ID */
      tDB2EnumNameMap mEnumNameMap;

      /* Enum entry map, indexed by enum ID/value pair */
      tDB2EnumEntryMap mEnumEntryMap;

      /* The assembled enum map */
      tDB2EnumMap mEnumMap;

      /* Parsed fragment modifier map - optional fragments */
      tDB2OptionalModMap mOptionalModMap;

      /* Parsed fragment modifier map - expression fragments */
      tDB2ExpressionModMap mExpressionModMap;

      /* Parsed fragment modifier map - element count specified arrays */
      tDB2Array1ModMap mArray1ModMap;

      /* Parsed fragment modifier map - start/stop index specified arrays */
      tDB2Array2ModMap mArray2ModMap;
};
