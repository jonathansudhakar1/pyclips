   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*               CLIPS Version 6.23  01/31/05          */
   /*                                                     */
   /*                  CLASS COMMANDS MODULE              */
   /*******************************************************/

/**************************************************************/
/* Purpose: Kernel Interface Commands for Object System       */
/*                                                            */
/* Principal Programmer(s):                                   */
/*      Brian L. Donnell                                      */
/*                                                            */
/* Contributing Programmer(s):                                */
/*                                                            */
/* Revision History:                                          */
/*      6.23: Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*            Changed name of variable log to logName        */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                            */
/**************************************************************/

/* =========================================
   *****************************************
               EXTERNAL DEFINITIONS
   =========================================
   ***************************************** */

#include <string.h>

#include "setup.h"

#if OBJECT_SYSTEM

#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "bload.h"
#endif

#include "argacces.h"
#include "classfun.h"
#include "classini.h"
#include "envrnmnt.h"
#include "modulutl.h"
#include "msgcom.h"
#include "router.h"

#define _CLASSCOM_SOURCE_
#include "classcom.h"

/* =========================================
   *****************************************
      INTERNALLY VISIBLE FUNCTION HEADERS
   =========================================
   ***************************************** */

#if (! BLOAD_ONLY) && (! RUN_TIME) && DEBUGGING_FUNCTIONS
static void SaveDefclass(void *,struct constructHeader *,void *);
#endif
static char *GetClassDefaultsModeName(unsigned short);

/* =========================================
   *****************************************
          EXTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

/*******************************************************************
  NAME         : EnvFindDefclass
  DESCRIPTION  : Looks up a specified class in the class hash table
                 (Only looks in current or specified module)
  INPUTS       : The name-string of the class (including module)
  RETURNS      : The address of the found class, NULL otherwise
  SIDE EFFECTS : None
  NOTES        : None
 ******************************************************************/
globle void *EnvFindDefclass(
  void *theEnv,
  char *classAndModuleName)
  {
   SYMBOL_HN *classSymbol = NULL;
   DEFCLASS *cls;
   struct defmodule *theModule = NULL;
   char *className;

   SaveCurrentModule(theEnv);
   className = ExtractModuleAndConstructName(theEnv,classAndModuleName);
   if (className != NULL)
     {
      classSymbol = FindSymbolHN(theEnv,ExtractModuleAndConstructName(theEnv,classAndModuleName));
      theModule = ((struct defmodule *) EnvGetCurrentModule(theEnv));
     }
   RestoreCurrentModule(theEnv);

   if (classSymbol == NULL)
     return(NULL);
   cls = DefclassData(theEnv)->ClassTable[HashClass(classSymbol)];
   while (cls != NULL)
     {
      if (cls->header.name == classSymbol)
        {
         if (cls->system || (cls->header.whichModule->theModule == theModule))
           return(cls->installed ? (void *) cls : NULL);
        }
      cls = cls->nxtHash;
     }
   return(NULL);
  }

/***************************************************
  NAME         : LookupDefclassByMdlOrScope
  DESCRIPTION  : Finds a class anywhere (if module
                 is specified) or in current or
                 imported modules
  INPUTS       : The class name
  RETURNS      : The class (NULL if not found)
  SIDE EFFECTS : Error message printed on
                  ambiguous references
  NOTES        : Assumes no two classes of the same
                 name are ever in the same scope
 ***************************************************/
globle DEFCLASS *LookupDefclassByMdlOrScope(
  void *theEnv,
  char *classAndModuleName)
  {
   DEFCLASS *cls;
   char *className;
   SYMBOL_HN *classSymbol;
   struct defmodule *theModule;

   if (FindModuleSeparator(classAndModuleName) == FALSE)
     return(LookupDefclassInScope(theEnv,classAndModuleName));

   SaveCurrentModule(theEnv);
   className = ExtractModuleAndConstructName(theEnv,classAndModuleName);
   theModule = ((struct defmodule *) EnvGetCurrentModule(theEnv));
   RestoreCurrentModule(theEnv);
   if(className == NULL)
     return(NULL);
   if ((classSymbol = FindSymbolHN(theEnv,className)) == NULL)
     return(NULL);
   cls = DefclassData(theEnv)->ClassTable[HashClass(classSymbol)];
   while (cls != NULL)
     {
      if ((cls->header.name == classSymbol) &&
          (cls->header.whichModule->theModule == theModule))
        return(cls->installed ? cls : NULL);
      cls = cls->nxtHash;
     }
   return(NULL);
  }

/****************************************************
  NAME         : LookupDefclassInScope
  DESCRIPTION  : Finds a class in current or imported
                   modules (module specifier
                   is not allowed)
  INPUTS       : The class name
  RETURNS      : The class (NULL if not found)
  SIDE EFFECTS : Error message printed on
                  ambiguous references
  NOTES        : Assumes no two classes of the same
                 name are ever in the same scope
 ****************************************************/
globle DEFCLASS *LookupDefclassInScope(
  void *theEnv,
  char *className)
  {
   DEFCLASS *cls;
   SYMBOL_HN *classSymbol;

   if ((classSymbol = FindSymbolHN(theEnv,className)) == NULL)
     return(NULL);
   cls = DefclassData(theEnv)->ClassTable[HashClass(classSymbol)];
   while (cls != NULL)
     {
      if ((cls->header.name == classSymbol) && DefclassInScope(theEnv,cls,NULL))
        return(cls->installed ? cls : NULL);
      cls = cls->nxtHash;
     }
   return(NULL);
  }

/******************************************************
  NAME         : LookupDefclassAnywhere
  DESCRIPTION  : Finds a class in specified
                 (or any) module
  INPUTS       : 1) The module (NULL if don't care)
                 2) The class name (module specifier
                    in name not allowed)
  RETURNS      : The class (NULL if not found)
  SIDE EFFECTS : None
  NOTES        : Does *not* generate an error if
                 multiple classes of the same name
                 exist as do the other lookup functions
 ******************************************************/
globle DEFCLASS *LookupDefclassAnywhere(
  void *theEnv,
  struct defmodule *theModule,
  char *className)
  {
   DEFCLASS *cls;
   SYMBOL_HN *classSymbol;

   if ((classSymbol = FindSymbolHN(theEnv,className)) == NULL)
     return(NULL);
   cls = DefclassData(theEnv)->ClassTable[HashClass(classSymbol)];
   while (cls != NULL)
     {
      if ((cls->header.name == classSymbol) &&
          ((theModule == NULL) ||
           (cls->header.whichModule->theModule == theModule)))
        return(cls->installed ? cls : NULL);
      cls = cls->nxtHash;
     }
   return(NULL);
  }

/***************************************************
  NAME         : DefclassInScope
  DESCRIPTION  : Determines if a defclass is in
                 scope of the given module
  INPUTS       : 1) The defclass
                 2) The module (NULL for current
                    module)
  RETURNS      : TRUE if in scope,
                 FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle BOOLEAN DefclassInScope(
  void *theEnv,
  DEFCLASS *theDefclass,
  struct defmodule *theModule)
  {
#if DEFMODULE_CONSTRUCT
   int moduleID;
   char *scopeMap;

   scopeMap = (char *) ValueToBitMap(theDefclass->scopeMap);
   if (theModule == NULL)
     theModule = ((struct defmodule *) EnvGetCurrentModule(theEnv));
   moduleID = (int) theModule->bsaveID;
   return(TestBitMap(scopeMap,moduleID) ? TRUE : FALSE);
#else
   return(TRUE);
#endif
  }

/***********************************************************
  NAME         : EnvGetNextDefclass
  DESCRIPTION  : Finds first or next defclass
  INPUTS       : The address of the current defclass
  RETURNS      : The address of the next defclass
                   (NULL if none)
  SIDE EFFECTS : None
  NOTES        : If ptr == NULL, the first defclass
                    is returned.
 ***********************************************************/
globle void *EnvGetNextDefclass(
  void *theEnv,
  void *ptr)
  {
   return((void *) GetNextConstructItem(theEnv,(struct constructHeader *) ptr,DefclassData(theEnv)->DefclassModuleIndex));
  }

/***************************************************
  NAME         : EnvIsDefclassDeletable
  DESCRIPTION  : Determines if a defclass
                   can be deleted
  INPUTS       : Address of the defclass
  RETURNS      : TRUE if deletable,
                 FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle BOOLEAN EnvIsDefclassDeletable(
  void *theEnv,
  void *ptr)
  {
#if (MAC_MCW || IBM_MCW) && (RUN_TIME || BLOAD_ONLY)
#pragma unused(theEnv,ptr)
#endif

#if BLOAD_ONLY || RUN_TIME
   return(FALSE);
#else
   DEFCLASS *cls;

#if BLOAD || BLOAD_AND_BSAVE
   if (Bloaded(theEnv))
     return(FALSE);
#endif
   cls = (DEFCLASS *) ptr;
   if (cls->system == 1)
     return(FALSE);
   return((IsClassBeingUsed(cls) == FALSE) ? TRUE : FALSE);
#endif
  }

/*************************************************************
  NAME         : UndefclassCommand
  DESCRIPTION  : Deletes a class and its subclasses, as
                 well as their associated instances
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : Syntax : (undefclass <class-name> | *)
 *************************************************************/
globle void UndefclassCommand(
  void *theEnv)
  {
   UndefconstructCommand(theEnv,"undefclass",DefclassData(theEnv)->DefclassConstruct);
  }

/********************************************************
  NAME         : EnvUndefclass
  DESCRIPTION  : Deletes the named defclass
  INPUTS       : None
  RETURNS      : TRUE if deleted, or FALSE
  SIDE EFFECTS : Defclass and handlers removed
  NOTES        : Interface for AddConstruct()
 ********************************************************/
globle BOOLEAN EnvUndefclass(
  void *theEnv,
  void *theDefclass)
  {
#if (MAC_MCW || IBM_MCW) && (RUN_TIME || BLOAD_ONLY)
#pragma unused(theEnv,theDefclass)
#endif

#if RUN_TIME || BLOAD_ONLY
   return(FALSE);
#else
   DEFCLASS *cls;

   cls = (DEFCLASS *) theDefclass;
#if BLOAD || BLOAD_AND_BSAVE
   if (Bloaded(theEnv))
     return(FALSE);
#endif
   if (cls == NULL)
     return(RemoveAllUserClasses(theEnv));
   return(DeleteClassUAG(theEnv,cls));
#endif
  }


#if DEBUGGING_FUNCTIONS

/*********************************************************
  NAME         : PPDefclassCommand
  DESCRIPTION  : Displays the pretty print form of
                 a class to the wdialog router.
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : Syntax : (ppdefclass <class-name>)
 *********************************************************/
globle void PPDefclassCommand(
  void *theEnv)
  {
   PPConstructCommand(theEnv,"ppdefclass",DefclassData(theEnv)->DefclassConstruct);
  }

/***************************************************
  NAME         : ListDefclassesCommand
  DESCRIPTION  : Displays all defclass names
  INPUTS       : None
  RETURNS      : Nothing useful
  SIDE EFFECTS : Defclass names printed
  NOTES        : H/L Interface
 ***************************************************/
globle void ListDefclassesCommand(
  void *theEnv)
  {
   ListConstructCommand(theEnv,"list-defclasses",DefclassData(theEnv)->DefclassConstruct);
  }

/***************************************************
  NAME         : EnvListDefclasses
  DESCRIPTION  : Displays all defclass names
  INPUTS       : 1) The logical name of the output
                 2) The module
  RETURNS      : Nothing useful
  SIDE EFFECTS : Defclass names printed
  NOTES        : C Interface
 ***************************************************/
globle void EnvListDefclasses(
  void *theEnv,
  char *logicalName,
  struct defmodule *theModule)
  {
   ListConstruct(theEnv,DefclassData(theEnv)->DefclassConstruct,logicalName,theModule);
  }

/*********************************************************
  NAME         : EnvGetDefclassWatchInstances
  DESCRIPTION  : Determines if deletions/creations of
                 instances of this class will generate
                 trace messages or not
  INPUTS       : A pointer to the class
  RETURNS      : TRUE if a trace is active,
                 FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : None
 *********************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle unsigned EnvGetDefclassWatchInstances(
  void *theEnv,
  void *theClass)
  {
#if MAC_MCW || IBM_MCW || MAC_XCD
#pragma unused(theEnv)
#endif

   return(((DEFCLASS *) theClass)->traceInstances);
  }

/*********************************************************
  NAME         : EnvSetDefclassWatchInstances
  DESCRIPTION  : Sets the trace to ON/OFF for the
                 creation/deletion of instances
                 of the class
  INPUTS       : 1) TRUE to set the trace on,
                    FALSE to set it off
                 2) A pointer to the class
  RETURNS      : Nothing useful
  SIDE EFFECTS : Watch flag for the class set
  NOTES        : None
 *********************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle void EnvSetDefclassWatchInstances(
  void *theEnv,
  unsigned newState,
  void *theClass)
  {
#if MAC_MCW || IBM_MCW || MAC_XCD
#pragma unused(theEnv)
#endif

   if (((DEFCLASS *) theClass)->abstract)
     return;
   ((DEFCLASS *) theClass)->traceInstances = newState;
  }

/*********************************************************
  NAME         : EnvGetDefclassWatchSlots
  DESCRIPTION  : Determines if changes to slots of
                 instances of this class will generate
                 trace messages or not
  INPUTS       : A pointer to the class
  RETURNS      : TRUE if a trace is active,
                 FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : None
 *********************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle unsigned EnvGetDefclassWatchSlots(
  void *theEnv,
  void *theClass)
  {
#if MAC_MCW || IBM_MCW || MAC_XCD
#pragma unused(theEnv)
#endif

   return(((DEFCLASS *) theClass)->traceSlots);
  }

/**********************************************************
  NAME         : EnvSetDefclassWatchSlots
  DESCRIPTION  : Sets the trace to ON/OFF for the
                 changes to slots of instances of the class
  INPUTS       : 1) TRUE to set the trace on,
                    FALSE to set it off
                 2) A pointer to the class
  RETURNS      : Nothing useful
  SIDE EFFECTS : Watch flag for the class set
  NOTES        : None
 **********************************************************/
#if IBM_TBC
#pragma argsused
#endif
globle void EnvSetDefclassWatchSlots(
  void *theEnv,
  unsigned newState,
  void *theClass)
  {
#if MAC_MCW || IBM_MCW || MAC_XCD
#pragma unused(theEnv)
#endif

   ((DEFCLASS *) theClass)->traceSlots = newState;
  }

/******************************************************************
  NAME         : DefclassWatchAccess
  DESCRIPTION  : Parses a list of class names passed by
                 AddWatchItem() and sets the traces accordingly
  INPUTS       : 1) A code indicating which trace flag is to be set
                    0 - Watch instance creation/deletion
                    1 - Watch slot changes to instances
                 2) The value to which to set the trace flags
                 3) A list of expressions containing the names
                    of the classes for which to set traces
  RETURNS      : TRUE if all OK, FALSE otherwise
  SIDE EFFECTS : Watch flags set in specified classes
  NOTES        : Accessory function for AddWatchItem()
 ******************************************************************/
globle unsigned DefclassWatchAccess(
  void *theEnv,
  int code,
  unsigned newState,
  EXPRESSION *argExprs)
  {
   if (code)
     return(ConstructSetWatchAccess(theEnv,DefclassData(theEnv)->DefclassConstruct,newState,argExprs,
                                    EnvGetDefclassWatchSlots,EnvSetDefclassWatchSlots));
   else
     return(ConstructSetWatchAccess(theEnv,DefclassData(theEnv)->DefclassConstruct,newState,argExprs,
                                    EnvGetDefclassWatchInstances,EnvSetDefclassWatchInstances));
  }

/***********************************************************************
  NAME         : DefclassWatchPrint
  DESCRIPTION  : Parses a list of class names passed by
                 AddWatchItem() and displays the traces accordingly
  INPUTS       : 1) The logical name of the output
                 2) A code indicating which trace flag is to be examined
                    0 - Watch instance creation/deletion
                    1 - Watch slot changes to instances
                 3) A list of expressions containing the names
                    of the classes for which to examine traces
  RETURNS      : TRUE if all OK, FALSE otherwise
  SIDE EFFECTS : Watch flags displayed for specified classes
  NOTES        : Accessory function for AddWatchItem()
 ***********************************************************************/
globle unsigned DefclassWatchPrint(
  void *theEnv,
  char *logName,
  int code,
  EXPRESSION *argExprs)
  {
   if (code)
     return(ConstructPrintWatchAccess(theEnv,DefclassData(theEnv)->DefclassConstruct,logName,argExprs,
                                      EnvGetDefclassWatchSlots,EnvSetDefclassWatchSlots));
   else
     return(ConstructPrintWatchAccess(theEnv,DefclassData(theEnv)->DefclassConstruct,logName,argExprs,
                                      EnvGetDefclassWatchInstances,EnvSetDefclassWatchInstances));
  }

#endif

/*********************************************************
  NAME         : GetDefclassListFunction
  DESCRIPTION  : Groups names of all defclasses into
                   a multifield variable
  INPUTS       : A data object buffer
  RETURNS      : Nothing useful
  SIDE EFFECTS : Multifield set to list of classes
  NOTES        : None
 *********************************************************/
globle void GetDefclassListFunction(
  void *theEnv,
  DATA_OBJECT_PTR returnValue)
  {
   GetConstructListFunction(theEnv,"get-defclass-list",returnValue,DefclassData(theEnv)->DefclassConstruct);
  }

/***************************************************************
  NAME         : EnvGetDefclassList
  DESCRIPTION  : Groups all defclass names into
                 a multifield list
  INPUTS       : 1) A data object buffer to hold
                    the multifield result
                 2) The module from which to obtain defclasses
  RETURNS      : Nothing useful
  SIDE EFFECTS : Multifield allocated and filled
  NOTES        : External C access
 ***************************************************************/
globle void EnvGetDefclassList(
  void *theEnv,
  DATA_OBJECT *returnValue,
  struct defmodule *theModule)
  {
   GetConstructList(theEnv,returnValue,DefclassData(theEnv)->DefclassConstruct,theModule);
  }

/*****************************************************
  NAME         : HasSuperclass
  DESCRIPTION  : Determines if class-2 is a superclass
                   of class-1
  INPUTS       : 1) Class-1
                 2) Class-2
  RETURNS      : TRUE if class-2 is a superclass of
                   class-1, FALSE otherwise
  SIDE EFFECTS : None
  NOTES        : None
 *****************************************************/
globle int HasSuperclass(
  DEFCLASS *c1,
  DEFCLASS *c2)
  {
   register unsigned i;

   for (i = 1 ; i < c1->allSuperclasses.classCount ; i++)
     if (c1->allSuperclasses.classArray[i] == c2)
       return(TRUE);
   return(FALSE);
  }

/********************************************************************
  NAME         : CheckClassAndSlot
  DESCRIPTION  : Checks class and slot argument for various functions
  INPUTS       : 1) Name of the calling function
                 2) Buffer for class address
  RETURNS      : Slot symbol, NULL on errors
  SIDE EFFECTS : None
  NOTES        : None
 ********************************************************************/
globle SYMBOL_HN *CheckClassAndSlot(
   void *theEnv,
   char *func,
   DEFCLASS **cls)
  {
   DATA_OBJECT temp;

   if (EnvArgTypeCheck(theEnv,func,1,SYMBOL,&temp) == FALSE)
     return(NULL);
   *cls = LookupDefclassByMdlOrScope(theEnv,DOToString(temp));
   if (*cls == NULL)
     {
      ClassExistError(theEnv,func,DOToString(temp));
      return(NULL);
     }
   if (EnvArgTypeCheck(theEnv,func,2,SYMBOL,&temp) == FALSE)
     return(NULL);
   return((SYMBOL_HN *) GetValue(temp));
  }

#if (! BLOAD_ONLY) && (! RUN_TIME)

/***************************************************
  NAME         : SaveDefclasses
  DESCRIPTION  : Prints pretty print form of
                   defclasses to specified output
  INPUTS       : The  logical name of the output
  RETURNS      : Nothing useful
  SIDE EFFECTS : None
  NOTES        : None
 ***************************************************/
globle void SaveDefclasses(
  void *theEnv,
  void *theModule,
  char *logName)
  {
#if DEBUGGING_FUNCTIONS
   DoForAllConstructsInModule(theEnv,theModule,SaveDefclass,DefclassData(theEnv)->DefclassModuleIndex,FALSE,(void *) logName);
#endif
  }

#endif

/* =========================================
   *****************************************
          INTERNALLY VISIBLE FUNCTIONS
   =========================================
   ***************************************** */

#if (! BLOAD_ONLY) && (! RUN_TIME) && DEBUGGING_FUNCTIONS

/***************************************************
  NAME         : SaveDefclass
  DESCRIPTION  : Writes out the pretty-print forms
                 of a class and all its handlers
  INPUTS       : 1) The class
                 2) The logical name of the output
  RETURNS      : Nothing useful
  SIDE EFFECTS : Class and handlers written
  NOTES        : None
 ***************************************************/
static void SaveDefclass(
  void *theEnv,
  struct constructHeader *theDefclass,
  void *userBuffer)
  {
   char *logName = (char *) userBuffer;
   unsigned hnd;
   char *ppForm;

   ppForm = EnvGetDefclassPPForm(theEnv,(void *) theDefclass);
   if (ppForm != NULL)
     {
      PrintInChunks(theEnv,logName,ppForm);
      EnvPrintRouter(theEnv,logName,"\n");
      hnd = EnvGetNextDefmessageHandler(theEnv,(void *) theDefclass,0);
      while (hnd != 0)
        {
         ppForm = EnvGetDefmessageHandlerPPForm(theEnv,(void *) theDefclass,hnd);
         if (ppForm != NULL)
           {
            PrintInChunks(theEnv,logName,ppForm);
            EnvPrintRouter(theEnv,logName,"\n");
           }
         hnd = EnvGetNextDefmessageHandler(theEnv,(void *) theDefclass,hnd);
        }
     }
  }

#endif

/***********************************************/
/* EnvSetClassDefaultsMode: Allows the setting */
/*    of the class defaults mode.              */
/***********************************************/
globle unsigned short EnvSetClassDefaultsMode(
  void *theEnv,
  unsigned short value)
  {
   unsigned short ov;

   ov = DefclassData(theEnv)->ClassDefaultsMode;
   DefclassData(theEnv)->ClassDefaultsMode = value;
   return(ov);
  }

/****************************************/
/* EnvGetClassDefaultsMode: Returns the */
/*    value of the class defaults mode. */
/****************************************/
globle unsigned short EnvGetClassDefaultsMode(
  void *theEnv)
  {
   return(DefclassData(theEnv)->ClassDefaultsMode);
  }

/***************************************************/
/* GetClassDefaultsModeCommand: H/L access routine */
/*   for the get-class-defaults-mode command.      */
/***************************************************/
globle void *GetClassDefaultsModeCommand(
  void *theEnv)
  {
   EnvArgCountCheck(theEnv,"get-class-defaults-mode",EXACTLY,0);

   return((SYMBOL_HN *) EnvAddSymbol(theEnv,GetClassDefaultsModeName(EnvGetClassDefaultsMode(theEnv))));
  }

/***************************************************/
/* SetClassDefaultsModeCommand: H/L access routine */
/*   for the set-class-defaults-mode command.      */
/***************************************************/
globle void *SetClassDefaultsModeCommand(
  void *theEnv)
  {
   DATA_OBJECT argPtr;
   char *argument;
   unsigned short oldMode;

   oldMode = DefclassData(theEnv)->ClassDefaultsMode;

   /*=====================================================*/
   /* Check for the correct number and type of arguments. */
   /*=====================================================*/

   if (EnvArgCountCheck(theEnv,"set-class-defaults-mode",EXACTLY,1) == -1)
     { return((SYMBOL_HN *) EnvAddSymbol(theEnv,GetClassDefaultsModeName(EnvGetClassDefaultsMode(theEnv)))); }

   if (EnvArgTypeCheck(theEnv,"set-class-defaults-mode",1,SYMBOL,&argPtr) == FALSE)
     { return((SYMBOL_HN *) EnvAddSymbol(theEnv,GetClassDefaultsModeName(EnvGetClassDefaultsMode(theEnv)))); }

   argument = DOToString(argPtr);

   /*=============================================*/
   /* Set the strategy to the specified strategy. */
   /*=============================================*/

   if (strcmp(argument,"conservation") == 0)
     { EnvSetClassDefaultsMode(theEnv,CONSERVATION_MODE); }
   else if (strcmp(argument,"convenience") == 0)
     { EnvSetClassDefaultsMode(theEnv,CONVENIENCE_MODE); }
   else
     {
      ExpectedTypeError1(theEnv,"set-class-defaults-mode",1,
      "symbol with value conservation or convenience");
      return((SYMBOL_HN *) EnvAddSymbol(theEnv,GetClassDefaultsModeName(EnvGetClassDefaultsMode(theEnv))));
     }

   /*===================================*/
   /* Return the old value of the mode. */
   /*===================================*/

   return((SYMBOL_HN *) EnvAddSymbol(theEnv,GetClassDefaultsModeName(oldMode)));
  }

/*******************************************************************/
/* GetClassDefaultsModeName: Given the integer value corresponding */
/*   to a specified class defaults mode, return a character string */
/*   of the class defaults mode's name.                            */
/*******************************************************************/
static char *GetClassDefaultsModeName(
  unsigned short mode)
  {
   char *sname;

   switch (mode)
     {
      case CONSERVATION_MODE:
        sname = "conservation";
        break;
      case CONVENIENCE_MODE:
        sname = "convenience";
        break;
      default:
        sname = "unknown";
        break;
     }

   return(sname);
  }

#endif
