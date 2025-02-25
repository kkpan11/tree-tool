// fasta2hash.cpp

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
*   Print hash codes for CDSs or proteins
*
*/


#undef NDEBUG

#include "../common.hpp"
using namespace Common_sp;
#include "seq.hpp"
using namespace Seq_sp;
#include "../version.inc"

#include "../common.inc"




namespace 
{
  
  

const StringVector gene_finders {"prodigal", "GeneMark"};


  
void reducePep (Peptide &pep)
{
  static const string s_Blast_Alphabet10 ("ABCBBFGHIIKIIBOPKKAAUIWXFB");  // By I.Tolstoy
  for (char& c : pep. seq)
    c = s_Blast_Alphabet10 [(size_t) (c - 'A')];
}



hash<string> str_hash;

unique_ptr<OFStream> targetSeqF;



void addHash (const string &s,
              const string &seqId,
              Vector<size_t> &hashes,
              Vector<size_t> &targetHashes)
{
	ASSERT (! s. empty ());
	const size_t h = str_hash (s);
  hashes << h;
  if (   targetSeqF. get ()
      && targetHashes. containsFast (h)
     )
  	*targetSeqF << seqId << endl;
}
  

  
struct ThisApplication final : Application
{
  ThisApplication ()
  : Application ("Print hash codes for CDSs or proteins")
  { 
    version = VERSION;
  	// Input
	  addPositional ("in", "Multi-FASTA file");
	  addKey ("gene_finder", "Gene finder used to find CDSs: " + gene_finders. toString (", "));
	  addFlag ("cds", "Are input sequences CDSs? If not then proteins");
	  addFlag ("segment", "Sequences are not full-length");  
	  addFlag ("translate", "Translate CDSs to proteins");
	  addFlag ("reduce", "Reduce 20 amino acids to 13 characters");
	  addKey ("ambig", "Max. number of ambiguous characters in sequences", "0");
    addKey ("kmer", "Cut the sequences into k-mers of this size. 0 - do not cut", "0");
	  addKey ("prot_len_min", "Min. protein length of input sequences", "0");
	  addKey ("hashes_max", "Max. number of hashes to print. 0 - print all", "0");
	  addKey ("target_hashes", "List of target hashes");
	  addFlag ("named", "Save hashes with sequence names");
	  // Output
	  addPositional ("out", "Output file of sorted unique hashes. Sorting is numeric.");
	  addKey ("out_prot", "File to save proteins with hashes");
	  addKey ("target_seq", "File to save the sequence names of the hashes in -target_hashes");
	}



	void body () const final
	{
    const string in            =               getArg ("in");  
    const string gene_finder   =               getArg ("gene_finder");
    const bool cds             =               getFlag ("cds");
    const bool segment         =               getFlag ("segment");
    const bool translate       =               getFlag ("translate");
    const bool reduce          =               getFlag ("reduce");
    const size_t ambig         = str2<size_t> (getArg ("ambig"));
    const size_t kmer          = str2<size_t> (getArg ("kmer"));
    const size_t prot_len_min  = str2<size_t> (getArg ("prot_len_min"));
    const size_t hashes_max    = str2<size_t> (getArg ("hashes_max"));
    const string target_hashes =               getArg ("target_hashes");
    const bool   named         =               getFlag ("named");
    const string out           =               getArg ("out"); 
    const string out_prot      =               getArg ("out_prot");
    const string target_seq    =               getArg ("target_seq");
    QC_ASSERT (! out. empty ()); 
    QC_IMPLY (cds, ! segment);
    QC_IMPLY (kmer, ! segment);
    QC_IMPLY (translate, cds);
    QC_IMPLY (reduce, ! cds || translate);
    QC_IMPLY (! out_prot. empty (), ! cds || translate);
    QC_IMPLY (! target_seq. empty (), ! target_hashes. empty ());
    if (! gene_finder. empty () && ! gene_finders. contains (gene_finder))
      throw runtime_error ("Uknown gene_finder: " + strQuote (gene_finder));
    

    Vector<size_t> targetHashes;
    if (! target_hashes. empty ())
    {
      LineInput f (target_hashes);  
  	  while (f. nextLine ())
    	  targetHashes << str2<size_t> (f. line);
    	cout << "# Target hashes: " << targetHashes. size () << endl;
    }
    targetHashes. sort ();
    QC_ASSERT (targetHashes. isUniq ());
    
    if (! target_seq. empty ())
      targetSeqF. reset (new OFStream (target_seq));
    

    const size_t seq_len_min = cds && ! translate 
                                 ? prot_len_min * 3 
                                 : prot_len_min;
        
    
    static_assert (sizeof (size_t) == 8, "Size of size_t must be 8 bytes");
    
    OFStream fOut (out);

    Vector<size_t> hashes;  hashes. reserve (10000);   // PAR
    size_t sequences = 0;
    unique_ptr<OFStream> protF;
    if (! out_prot. empty ())
      protF. reset (new OFStream (out_prot));
    {
		  Multifasta f (in, ! cds);
		  while (f. next ())
		  {
		    string s;
		    string seqId;
		    if (cds)
		    {
  		    const Dna dna (f, 10000, false);  // PAR
  		    seqId = dna. getId ();
		    	dna. qc (); 
  		    if (gene_finder == "GeneMark" && ! contains (dna. name, " trunc5:0 trunc3:0"))  
  		      continue;
  		    if (gene_finder == "prodigal" && ! contains (dna. name, ";partial=00"))  
  		      continue;
  		    if (dna. getXs () > ambig)
  		      continue;
  		    if (translate)
  		    {
    		    Peptide pep (dna. cds2prot (11, false, false, true, false));  // gencode - PAR ??
    		    pep. qc ();
    		    QC_ASSERT (! pep. seq. empty ());
    		  #if 0
    		    if (gene_finder. empty () && pep. seq. front () != 'm')   // LIV ??
    		      continue;
    		  #endif
    		    QC_ASSERT (pep. seq. front () == 'm');
					  QC_IMPLY (! ambig, ! pep. getXs ());  
    	      strUpper (pep. seq);  // Start codons are lowercased
	  		    if (protF. get ())
	  		    	pep. saveText (*protF);
    	      if (reduce)
    	        reducePep (pep);
    	      s = pep. seq;
    	    }
  		    else
  		      s = dna. seq;
	  		}
    		else
    		{
  		    Peptide pep (f, 1000, false);  // PAR
  		    seqId = pep. getId ();
  		    pep. qc ();
  		    if (! segment)
  		    {
    		    if (gene_finder == "GeneMark" && ! contains (pep. name, " trunc5:0 trunc3:0"))  
    		      continue;
    		    if (gene_finder == "prodigal" && ! contains (pep. name, ";partial=00"))  
    		      continue;
    		    if (gene_finder. empty () && pep. seq. front () != 'M')  
    		      continue;
    		  }
  		    pep. trimStop ();
   	      strUpper (pep. seq);  // Start codons are lowercased
  		    QC_ASSERT (! pep. seq. empty ());
  		    QC_IMPLY (! segment, pep. seq. front () == 'M');
  		    if (pep. getXs () > ambig)
  		      continue;
  		    if (contains (pep. seq, '*'))
  		      throw runtime_error ("Protein " + pep. name + " contains a stop codon");
  		    if (protF. get ())
  		    	pep. saveText (*protF);
  	      if (reduce)
   	        reducePep (pep);
		      s = pep. seq;
    		}
	      if (s. size () < seq_len_min)
	      	continue;

	      sequences++;
        if (kmer)
          FOR (size_t, i, s. size ())
          {
            const string sub (s. substr (i, kmer));
            if (sub. size () < kmer)
              break;
            addHash (sub, seqId, hashes, targetHashes);
          }
        else
          if (named)
            fOut << seqId << '\t' << str_hash (s) << endl;
          else
            addHash (s, seqId, hashes, targetHashes);
	    }
    }
    cout << "Good sequences: " << sequences << endl;
    if (! named)
    {
      cout << "All hashes: " << hashes. size () << endl;
      hashes. sort ();
      hashes. uniq ();
      cout << "Unique hashes: " << hashes. size () << endl;    
      {
        if (hashes_max)
        {
          const size_t size = min (hashes. size (), hashes_max);
          FOR (size_t, i, size)
            fOut << hashes [i] << endl;
        }
        else
          for (const size_t h : hashes)
            fOut << h << endl;
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



