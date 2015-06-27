/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2012 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed-code.org for more information.

   This file is part of plumed, version 2.0.

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
#include "DimensionalityReductionBase.h"
#include "reference/ReferenceConfiguration.h"
#include "reference/MetricRegister.h"

namespace PLMD {
namespace dimred {

void DimensionalityReductionBase::registerKeywords( Keywords& keys ){
  analysis::AnalysisWithAnalysableOutput::registerKeywords( keys );
  keys.add("compulsory","NLOW_DIM","number of low-dimensional coordinates required");
  keys.addFlag("RUN_ON_PROJECTION",false,"run the dimensionality reduction algorithm on the projection generated by the input dimensionality reduction object");
  keys.remove("RUN"); keys.remove("STRIDE"); keys.remove("USE_ALL_DATA"); 
  keys.remove("ATOMS"); keys.remove("METRIC"); keys.remove("FMT"); keys.remove("REWEIGHT_BIAS");
  keys.remove("TEMP"); keys.remove("REWEIGHT_TEMP"); keys.remove("WRITE_CHECKPOINT"); 
}

DimensionalityReductionBase::DimensionalityReductionBase( const ActionOptions& ao ):
Action(ao),
analysis::AnalysisWithAnalysableOutput(ao),
dimredbase(NULL)
{
  parse("NLOW_DIM",nlow); parseFlag("RUN_ON_PROJECTION",use_dimred_dissims);
  if( nlow<1 ) error("dimensionality of low dimensional space must be at least one");
  log.printf("  projecting in %d dimensional space \n",nlow);

  // Now we check if the input was a dimensionality reduction object
  dimredbase = dynamic_cast<DimensionalityReductionBase*>( dimredstash );
  if( use_dimred_dissims && !dimredbase ) error("am supposed to be doing dimensionality reduction based on input projection but input is not a projection");

  // Check that some dissimilarity information is available
  if( !dissimilaritiesWereSet() ) error("dissimilarities have not been calcualted in input action");

  ReferenceConfigurationOptions("EUCLIDEAN");
  mydata=metricRegister().create<ReferenceConfiguration>("EUCLIDEAN");
  std::vector<std::string> dimnames(nlow); std::string num;
  for(unsigned i=0;i<nlow;++i){ Tools::convert(i+1,num); dimnames[i] = getLabel() + "." + num; }
  mydata->setNamesAndAtomNumbers( std::vector<AtomNumber>(), dimnames );
}

DimensionalityReductionBase::~DimensionalityReductionBase(){
  delete mydata;
}

ReferenceConfiguration* DimensionalityReductionBase::getOutputConfiguration( const unsigned& idata ){
  std::vector<double> pp(nlow); for(unsigned i=0;i<nlow;++i) pp[i]=projections(idata,i);
  std::vector<double> empty( pp.size() );
  mydata->setReferenceConfig( std::vector<Vector>(), pp, empty );
  return mydata;
}

double DimensionalityReductionBase::getInputDissimilarity( const unsigned& idata, const unsigned& jdata ){
  if( dimredbase && use_dimred_dissims ) return dimredbase->getOutputDissimilarity( idata, jdata );
  if( dimredbase ) return dimredbase->getInputDissimilarity( idata, jdata );
  return getDissimilarity( idata, jdata );
}

void DimensionalityReductionBase::getOutputForPoint( const unsigned& idata, std::vector<double>& point ){
  if( point.size()!=nlow ) point.resize( nlow );
  for(unsigned i=0;i<nlow;++i) point[i]=projections(idata,i);
}

double DimensionalityReductionBase::getOutputDissimilarity( const unsigned& idata, const unsigned& jdata ){
  double dissim=0; for(unsigned i=0;i<nlow;++i){ double tmp=projections(idata,i)-projections(jdata,i); dissim+=tmp*tmp; }
  return dissim;
}

void DimensionalityReductionBase::performAnalysis(){
  // Resize the projections array
  projections.resize( getNumberOfDataPoints(), nlow );
  // Calculate matrix of dissimilarities
  Matrix<double> targets( getNumberOfDataPoints(), getNumberOfDataPoints() );
  for(unsigned i=1;i<getNumberOfDataPoints();++i){
     for(unsigned j=0;j<i;++j) targets(i,j)=targets(j,i)=getInputDissimilarity( i, j );
  }
  // This calculates the projections of the points
  calculateProjections( targets, projections );
}

}
}
