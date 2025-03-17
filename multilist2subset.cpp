// multilist2subset.cpp

/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE                          
*               National Center for Biotechnology Information
*                                                                          
*  This software/database is a "United States Government Work" under the   
*  terms of the United States Copyright Act.  It was written as part of    
*  the author's official duties as a United States Government employee and 
*  thus cannot be copyrighted.  This software/database is freely available 
*  to the public for use. The National Library of Medicine and the U.S.    
*  Government have not placed any restriction on its use or reproduction.  
*                                                                          
*  Although all reasonable efforts have been taken to ensure the accuracy  
*  and reliability of the software and data, the NLM and the U.S.          
*  Government do not and cannot warrant the performance or results that    
*  may be obtained by using this software or data. The NLM and the U.S.    
*  Government disclaim all warranties, express or implied, including       
*  warranties of performance, merchantability or fitness for any particular
*  purpose.                                                                
*                                                                          
*  Please cite the author in any work or product based on this material.   
*
* ===========================================================================
*
* Author: Vyacheslav Brover
*
* File Description:
*   Print the top N different items
*
*/


#undef NDEBUG

#include "common.hpp"
using namespace Common_sp;
#include "version.inc"

#include "common.inc"



namespace 
{


struct ThisApplication final : Application
{
  ThisApplication ()
    : Application ("Print the top N different items")
  	{
      version = VERSION;
  	  addPositional ("list", "List with possible duplications");
  	  addPositional ("subset", "Number of items to print");
  	}



	void body () const final
	{
		const string listFName = getArg ("list");
		const size_t subset    = (size_t) arg2uint ("subset");
		
		
    size_t n = 0;
    unordered_set<string> items;
    items. rehash (subset);
    LineInput f (listFName);
    while (f. nextLine () && n < subset)
    {
      trim (f. line);
      if (items. insert (f. line). second)
      {
        cout << f. line << '\n';
        n++;
      }
    }
	}
};



}  // namespace



int main (int argc, 
          const char* argv[])
{
  ThisApplication app;
  return app. run (argc, argv);
}
