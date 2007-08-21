/*
 * This software, which is provided in confidence, was prepared by employees of
 * Pacific Northwest National Laboratory operated by Battelle Memorial Institute.
 * Battelle has certain unperfected rights in the software which should not be
 * copied or otherwise disseminated outside your organization without the express
 * written authorization from Battelle. All rights to the software are reserved
 * by Battelle. Battelle makes no warranty, express or implied, and assumes no
 * liability or responsibility for the use of this software.
 */

/*!
 * \file aggr_emissions_coef.h
 * \ingroup Objects
 * \brief AggrEmissionsCoef header file.
 * \author Jim Naslund
 */

#include "util/base/include/definitions.h"
#include "containers/include/scenario.h"
#include "util/base/include/model_time.h"

#include "emissions/include/aggr_emissions_coef.h"
#include "util/logger/include/ilogger.h"
#include "containers/include/iinfo.h"
#include "emissions/include/total_sector_emissions.h"

using namespace std;

extern Scenario* scenario;

//! Clone operator.
AggrEmissionsCoef* AggrEmissionsCoef::clone() const {
    return new AggrEmissionsCoef( *this );
}

void AggrEmissionsCoef::initCalc( const IInfo* aSubsectorInfo, const string& aName, const int aPeriod ){

    int dataYear = aSubsectorInfo->getDouble( TotalSectorEmissions::aggrEmissionsYearPrefix() + aName, true );
    
    // First check that are at the data year for the aggregate emissions, if not do nothing
    const Modeltime* modeltime = scenario->getModeltime();
    if ( modeltime->getyr_to_per( dataYear ) == aPeriod ) {
        // Read the emissions coef. from the model and print a warning if it overwrote something.
        if( aSubsectorInfo->hasValue( TotalSectorEmissions::aggrEmissionsPrefix() + aName ) ){
            mEmissionsCoef = aSubsectorInfo->getDouble( TotalSectorEmissions::aggrEmissionsPrefix() + aName,
                                                    true );
        }
        else {
            ILogger& mainLog = ILogger::getLogger( "main_log" );
            mainLog.setLevel( ILogger::WARNING );
            mainLog << "Aggregate GHG object " << aName << " has no emissions data supplied." << endl;
        }
    }
}

const string& AggrEmissionsCoef::getXMLName() const{
    static const string XML_NAME = "emisscoef";
    return XML_NAME;
}


