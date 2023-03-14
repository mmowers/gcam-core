import pandas as pd
from pdb import set_trace as b
import os

results_folder = '/home/mmowers/GCAM/run_results_2023-03-07'
outputs_folder = f'{results_folder}/csv_results'
os.mkdir(outputs_folder)
scens = ['core','plcoe']
concat_dct = {} #key is the name of the output, and value is a list of dataframes to be concatenated (each scenario)
for scen in scens:
    print(f'Gathering results from {scen}')
    df = pd.read_csv(f'{results_folder}/queryout_{scen}.csv', header=None, sep='\n')
    df = df[0].str.split(',', expand=True) #Unfortunately the raw data has [scenario],[date] in the "scenario" column. See HACK below
    df_nm = df[df[1].isnull()][0].copy() #series of table names and associated index
    df_nm[len(df)] = 'end' #Add the index of the end of the dataframe
    for i in range(len(df_nm) - 1): #minus 1 because the final entry is just 'end'
        df_res = df.iloc[df_nm.index[i]:df_nm.index[i+1],:].copy()
        #Find name of table and add it if it doesn't already exist
        name = df_res.iloc[0,0]
        if name not in concat_dct:
            concat_dct[name] = []
        #Remove first row (with just the name)
        df_res = df_res.iloc[1:,:]
        #Make the next row the header of the table and then remove this row
        df_res.columns = df_res.iloc[0]
        df_res = df_res.iloc[1:,:]
        #HACK: Shift columns because original "scenario" column looked like "[scenario],[date]"
        cols = df_res.columns.tolist()
        cols.insert(1,'date')
        cols.pop()
        df_res.columns = cols
        #Remove columns that are named None or empty strings
        cols = [c for c in cols if c not in ['',None]]
        df_res = df_res[cols].copy()
        #Add the scenario name we'll be using in outputs
        df_res['scen_name'] = scen
        #melt the years
        id_vars = [c for c in df_res.columns if not c.isnumeric()]
        df_res = pd.melt(df_res, id_vars=id_vars, var_name='year', value_name='value')
        concat_dct[name].append(df_res)

for name in concat_dct:
    print(f'Outputting {name}')
    output = pd.concat(concat_dct[name], ignore_index=True)
    output.to_csv(f'{outputs_folder}/{name}.csv', index=False)