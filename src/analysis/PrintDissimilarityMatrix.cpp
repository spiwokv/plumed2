/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2013,2014 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed-code.org for more information.

   This file is part of plumed, version 2.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#include "tools/OFile.h"
#include "Analysis.h"
#include "core/ActionRegister.h"

namespace PLMD {
namespace analysis {

class PrintDissimilarityMatrix : public Analysis {
private:
  std::string fname;
public:
  static void registerKeywords( Keywords& keys );
  PrintDissimilarityMatrix( const ActionOptions& ao );
  void performAnalysis();
  void performTask(){ plumed_error(); }
};

PLUMED_REGISTER_ACTION(PrintDissimilarityMatrix,"PRINT_DISSIMILARITY_MATRIX")

void PrintDissimilarityMatrix::registerKeywords( Keywords& keys ){
  Analysis::registerKeywords( keys );
  keys.add("compulsory","FILE","name of file on which to output the data");
}

PrintDissimilarityMatrix::PrintDissimilarityMatrix( const ActionOptions& ao ):
Action(ao),
Analysis(ao)
{
  parseOutputFile("FILE",fname); 
  log.printf("  printing to file named %s with formt %s \n",fname.c_str(), getOutputFormat().c_str() );
}

void PrintDissimilarityMatrix::performAnalysis(){
  std::string ofmt=" "+getOutputFormat();
  OFile ofile; ofile.setBackupString("analysis"); ofile.open(fname); 
  for(unsigned i=0;i<getNumberOfDataPoints();++i){
      for(unsigned j=0;j<getNumberOfDataPoints();++j) ofile.printf(ofmt.c_str(), getDissimilarity( i,j ) );
      ofile.printf("\n");
  }   
  ofile.close();
}

}
}
