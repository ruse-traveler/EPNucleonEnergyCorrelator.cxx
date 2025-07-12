// ============================================================================
//! \file   Calculator.hxx
//! \author Derek Anderson
//! \date   07.11.2025
// ----------------------------------------------------------------------------
//! Calculates NEC and saves them to an RNTuple for
//! histogramming downstream.
// ============================================================================

/* TODO includes go here */



namespace EPNucleonEnergyCorrelator {

  // ==========================================================================
  //! NEC Calculator
  // --------------------------------------------------------------------------
  //! Class to process extracted reconstructed and generated
  //! particles and compute NECs. Computations are stored
  //! in an RNTuple to be histogrammed downstream.
  // ==========================================================================
  class Calculator {

    public:

      // ctor/dtor
      Calculator()  {};
      ~Calculator() {};

      // interface
      void Init();
      void Run();
      void End();

    private:

      /* TODO fill in */

  };  // end Calculator

}  // end EPNucleonEnergyCorrelator namespace

// end =======================================================================
