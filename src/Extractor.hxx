// ============================================================================
//! \file   Extractor.hxx
//! \author Derek Anderson
//! \date   07.11.2025
// ----------------------------------------------------------------------------
//! Skims EICrecon output to extract necessary 
//! info.
// ============================================================================

/* TODO includes go here */



namespace EPNucleonEnergyCorrelator {

  // ==========================================================================
  //! NEC Extractor
  // --------------------------------------------------------------------------
  //! Class to process EICrecon output and extract only 
  //! necessary information. Extracted information is
  //! saved in an RNTuple to be processed downstream.
  // ==========================================================================
  class Extractor {

    public:

      // ctor/dtor
      Extractor()  {};
      ~Extractor() {};

      // interface
      void Init();
      void Run();
      void End();

    private:

      /* TODO fill in */

  };  // end Extractor

}  // end EPNucleonEnergyCorrelator namespace

// end =======================================================================
