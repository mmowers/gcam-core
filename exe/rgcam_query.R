#Use batch_queries.xml to extract data from scenario, and save each query as a csv file.
#Note, rgcam will need to have been cloned into gcam/exe prior to calling this script.

library('devtools')
#Uncomment the following line to install rgcam from github (takes forever tho)
#install_github('JGCRI/rgcam', build_vignettes=TRUE)

# Load rgcam
devtools::load_all('./rgcam')

options("rgcam.saved_compressed" = FALSE)

DATE <- gsub("-", ".", Sys.Date())

# Connect a GCAM database using rgcam
conn <- localDBConn(dbPath = '../output', dbFile = 'database_basexdb', maxMemory = '8g')

# Run batch query on new scenarios and add to prj (if needed)
prj <- addScenario(conn, paste0('run_', DATE, '.dat'), queryFile = './batch_queries.xml')

# Produce list of queries in data set
scenarios <- listScenarios(prj)
queries <- listQueries(prj, scenarios[[1]])

# Loop over queries, extract data, write to csv
for (q in queries) {
  data <- getQuery(prj, q)
  write.csv(data, paste0("results/",q, ".csv"), row.names = F)
}

# # END
