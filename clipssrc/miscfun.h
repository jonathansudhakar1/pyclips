   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.23  01/31/05            */
   /*                                                     */
   /*          MISCELLANEOUS FUNCTIONS HEADER FILE        */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*      6.23: Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*************************************************************/

#ifndef _H_miscfun

#define _H_miscfun

#ifdef LOCALE
#undef LOCALE
#endif

#ifdef _MISCFUN_SOURCE_
#define LOCALE
#else
#define LOCALE extern
#endif

   LOCALE void                           MiscFunctionDefinitions(void *);
   LOCALE void                           CreateFunction(void *,DATA_OBJECT_PTR);
   LOCALE long int                       SetgenFunction(void *);
   LOCALE void                          *GensymFunction(void *);
   LOCALE void                          *GensymStarFunction(void *);
   LOCALE long                           RandomFunction(void *);
   LOCALE void                           SeedFunction(void *);
   LOCALE long int                       LengthFunction(void *);
   LOCALE void                           ConserveMemCommand(void *);
   LOCALE long int                       ReleaseMemCommand(void *);
   LOCALE long int                       MemUsedCommand(void *);
   LOCALE long int                       MemRequestsCommand(void *);
   LOCALE void                           OptionsCommand(void *);
   LOCALE void                           ExpandFuncCall(void *,DATA_OBJECT *);
   LOCALE void                           DummyExpandFuncMultifield(void *,DATA_OBJECT *);
   LOCALE void                          *CauseEvaluationError(void *);
   LOCALE BOOLEAN                        SetSORCommand(void *);
   LOCALE void                          *GetFunctionRestrictions(void *);
   LOCALE void                           AproposCommand(void *);
   LOCALE void                          *GensymStar(void *);
   LOCALE void                           GetFunctionListFunction(void *,DATA_OBJECT *);
   LOCALE void                           FuncallFunction(void *,DATA_OBJECT *);
   LOCALE double                         TimerFunction(void *);
   LOCALE double                         TimeFunction(void *);

#endif






