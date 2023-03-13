# LEGAL NOTICE
# This computer software was prepared by Battelle Memorial Institute,
# hereinafter the Contractor, under Contract No. DE-AC05-76RL0 1830
# with the Department of Energy (DOE). NEITHER THE GOVERNMENT NOR THE
# CONTRACTOR MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY
# LIABILITY FOR THE USE OF THIS SOFTWARE. This notice including this
# sentence must appear on any copies of this computer software.
#
# EXPORT CONTROL
# User agrees that the Software will not be shipped, transferred or
# exported into any country or used in any manner prohibited by the
# United States Export Administration Act or any other applicable
# export laws, restrictions or regulations (collectively the "Export Laws").
# Export of the Software may require some form of license or other
# authority from the U.S. Government, and failure to obtain such
# export control license may result in criminal liability under
# U.S. laws. In addition, if the Software is identified as export
# controlled items under the Export Laws, User represents and warrants
# that User is not a citizen, or otherwise located within, an embargoed
# nation (including without limitation Iran, Syria, Sudan, Cuba, and
# North Korea) and that User is not otherwise prohibited under the
# Export Laws from receiving the Software.
#
# Copyright 2011 Battelle Memorial Institute.  All Rights Reserved.
# Distributed as open-source under the terms of the Educational Community
# License version 2.0 (ECL 2.0). http://www.opensource.org/licenses/ecl2.php
#
# For further details, see: http://www.globalchange.umd.edu/models/gcam/


# rgcam_query.R
#
# A script for querying data from multiple GCAM databases via rgcam and for-loop.
# 
# Matthew Binsted, August 2017


# Load rgcam (if rgam is not installed, use install_github('JGCRI/rgcam') )
library('devtools')
devtools::load_all('/path/to/rgcam')


# ============================================================================

run_numbers <- c(0:3)

for (n in run_numbers) {

  # Connect a GCAM database using rgcam
  conn <- localDBConn(dbPath = paste0('path/to/run_', n, '/db'), dbFile = 'database_basexdb', maxMemory = '8g')

  # Run batch query on new scenarios and add to prj (if needed)
  prj <- addScenario(conn, proj =  'data_file_name.dat', queryFile = 'batch_queries.xml')

}


# ============================================================================
# DONE
