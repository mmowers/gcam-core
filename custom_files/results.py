import pandas as pd
import os
import shutil
import json
import requests
import re
from pdb import set_trace as b

#user switches
output_csvs = False
base_folder = '/home/mmowers/GCAM/reports' #must exist

this_dir_path = os.path.dirname(os.path.realpath(__file__))

outputs_folder = f'{base_folder}/vizit_report' #must not exist
os.mkdir(outputs_folder) #Will throw an error if outputs_folder already exists
shutil.copy2(f'{this_dir_path}/results.py',outputs_folder)
shutil.copy2(f'{this_dir_path}/scenario_styles.csv',outputs_folder)
scens = pd.read_csv(f'{this_dir_path}/scenario_styles.csv')
ignore_results = {
    'global': [
        'CO2 emissions by sector.csv',
        'inputs by tech.csv',
        'prices of all markets.csv',
    ],
    'usa':[
    ]
}
filters = {
    'global': {
        'inputs by tech.csv': {'sector':['electricity']},
        'prices of all markets.csv': {'market':['USAwind-trial-supply', 'USAsolar-trial-supply','USAwind_offshore-trial-supply']},
        'costs by tech and input.csv': {'sector':['electricity'], 'region':['USA']},
        'elec gen costs by tech.csv': {'sector':['electricity'],  'region':['USA']},
        'elec gen by gen tech and cooling tech (new).csv': {'sector':['electricity']},
        # "elec share-weights by subsector.csv": {"region":["USA"]},
    },
    'usa': {
    }
}

concat_dct = {} #key is the name of the output, and value is a list of dataframes to be concatenated (each scenario)
for index, scen in scens.iterrows():
    print(f'\nGathering results from {scen.column_value}')
    #Loop through csv files in the scenario folder that aren't in ignore_results, filter with filters, add them to concat_dct
    gcam_scope = scen.gcam_scope
    for file in os.listdir(f'{scen.path}/results'):
        if file.endswith('.csv') and file not in ignore_results[gcam_scope]:
            print(f'processing {file}')
            df = pd.read_csv(f'{scen.path}/results/{file}')
            if file in filters[gcam_scope]:
                for key,val in filters[gcam_scope][file].items():
                    df = df[df[key].isin(val)].copy()
            #Sort by specified columns
            sort_cols = [c for c in ['subsector','subsector...5','year'] if c in df]
            df = df.sort_values(sort_cols)
            df['scen_name'] = scen.column_value
            if file not in concat_dct:
                concat_dct[file] = []
            concat_dct[file].append(df)

print('\nConcatenating results and adding differences')
data_dict = {}
for name in concat_dct:
    print(f'processing {name}')
    df = pd.concat(concat_dct[name], ignore_index=True)
    val_col = 'value'
    df['value'] = pd.to_numeric(df['value'])
    idx_cols = [c for c in df if c not in [val_col, 'scen_name']]

    #Add scenario component columns
    df[['version','policy']] = df['scen_name'].str.split(expand=True)
    #Find differences from Core
    df_ref = df[df['version'] == 'core'].copy()
    df_ref = df_ref[['policy'] + idx_cols + [val_col]].copy()
    df_ref = df_ref.rename(columns={val_col: f'{val_col} Core'})
    df = df.merge(df_ref, how='outer', on=['policy'] + idx_cols)
    df[[val_col, f'{val_col} Core']] = df[[val_col, f'{val_col} Core']].fillna(0)

    df[f'{val_col} Diff with Core'] = df[val_col] - df[f'{val_col} Core']
    df[f'{val_col} % Diff with Core'] = 100 * df[f'{val_col} Diff with Core'] / df[f'{val_col} Core']
    val_cols = [val_col,f'{val_col} Diff with Core',f'{val_col} % Diff with Core']
    out_cols = ['scen_name','version','policy'] + idx_cols + val_cols
    df = df[out_cols].copy()

    if name == 'elec gen by gen tech.csv':
        #Add fraction of total global generation.
        df_tot = df.groupby(['scen_name','year'])['value'].sum().reset_index().rename(columns={'value':'val_tot'})
        df = df.merge(df_tot, on=['scen_name','year'], how='left')
        df['val_frac'] = df['value']/df['val_tot']

    #Add to data_dict
    data_dict[name] = df.to_dict(orient='list')

    if output_csvs:
        df.to_csv(f'{outputs_folder}/{name}', index=False)

print('\nOutputting results')
df_style = scens[['column_name','column_value','color']].copy()
data_dict['scenario_styles.csv'] = df_style.to_dict(orient='list')
with open(f'{this_dir_path}/vizit-config_{gcam_scope}.json') as json_file:
    #Note that this uses the final gcam_scope defined in the scenarios csv file, but that should be consistent across scenarios.
    vizit_config = json.load(json_file)
vizit_commit = '7011d363e40386264bedb3155629729b225fd22e'
vizit_url = f'https://raw.githubusercontent.com/mmowers/vizit/{vizit_commit}/index.html'
f_out_str = requests.get(vizit_url).text
data_str = json.dumps(data_dict, separators=(',',':'))
config_str = json.dumps(vizit_config, separators=(',',':'))
f_out_str = re.sub('let config_load = .*;\n', f'let config_load = {config_str};\n', f_out_str, 1)
f_out_str = re.sub('let rawData = .*;\n', f'let rawData = {data_str};\n', f_out_str, 1)
with open(f'{outputs_folder}/report_vizit.html', 'w') as f_out:
    f_out.write(f_out_str)

print('\nDone!')